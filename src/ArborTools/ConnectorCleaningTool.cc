/// \file ConnectorCleaningTool.cc
/*
 *
 * ConnectorCleaningTool.cc source template automatically generated by a class generator
 * Creation date : mar. d�c. 8 2015
 *
 * This file is part of ArborContent libraries.
 * 
 * ArborContent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * ArborContent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ArborContent.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#include "ArborTools/ConnectorCleaningTool.h"

#include "Pandora/AlgorithmHeaders.h"

#include "ArborObjects/CaloHit.h"
#include "ArborObjects/Connector.h"
#include "ArborHelpers/CaloHitHelper.h"

namespace arbor_content
{

  pandora::StatusCode ConnectorCleaningTool::Process(const pandora::Algorithm &/*algorithm*/, const pandora::CaloHitList *const pCaloHitList)
  {
    if(pCaloHitList->empty())
      return pandora::STATUS_CODE_SUCCESS;

    if(0 == m_strategy)
    {
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->CleanCaloHits(pCaloHitList));
    }
    else
    {
      pandora::OrderedCaloHitList orderedCaloHitList;
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, orderedCaloHitList.Add(*pCaloHitList));

      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->CleanCaloHits(orderedCaloHitList));
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  float ConnectorCleaningTool::GetOrderParameter(const arbor_content::Connector *const pConnector, const pandora::CartesianVector &referenceDirection) const
  {
    const pandora::CartesianVector connectorVector = pConnector->GetVector(BACKWARD_DIRECTION);
    const float angle = referenceDirection.GetOpeningAngle(connectorVector)/M_PI;
    const float distance = pConnector->GetNormalizedLength();

    return (std::pow(angle, m_orderParameterAnglePower) * std::pow(distance, m_orderParameterDistancePower));
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::CartesianVector ConnectorCleaningTool::GetReferenceVector(const arbor_content::CaloHit *const pCaloHit) const
  {
    pandora::CartesianVector referenceVector = pandora::CartesianVector(0, 0, 0);

    try
    {
      pandora::CartesianVector meanForwardDirection(0, 0, 0);
      pandora::CartesianVector meanBackwardDirection(0, 0, 0);

      PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, CaloHitHelper::GetMeanDirection(pCaloHit, FORWARD_DIRECTION,
          meanForwardDirection, m_forwardReferenceDirectionDepth));
      meanForwardDirection *= m_forwardConnectorWeight;

      PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, CaloHitHelper::GetMeanDirection(pCaloHit, BACKWARD_DIRECTION,
          meanBackwardDirection, m_backwardReferenceDirectionDepth));

      meanBackwardDirection *= m_backwardConnectorWeight;

      if(meanForwardDirection == meanBackwardDirection)
        throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);

      referenceVector = meanBackwardDirection - meanForwardDirection;
    }
    catch(pandora::StatusCodeException &exception)
    {
    }

    return referenceVector;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ConnectorCleaningTool::CleanCaloHits(const pandora::CaloHitList *const pCaloHitList) const
  {
    CaloHitCleaningMap caloHitCleaningMap;

    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), endIter = pCaloHitList->end() ;
        endIter != iter ; ++iter)
    {
      const arbor_content::CaloHit *pCaloHit = dynamic_cast<const arbor_content::CaloHit *>(*iter);

      if(NULL == pCaloHit)
        return pandora::STATUS_CODE_FAILURE;

      const ConnectorList &backwardConnectorList(ArborContentApi::GetConnectorList(pCaloHit, BACKWARD_DIRECTION));

      if(backwardConnectorList.size() < 2)
        continue;

      pandora::CartesianVector referenceVector(this->GetReferenceVector(pCaloHit));

      // undefined reference vector ?
      if(referenceVector == pandora::CartesianVector(0.f, 0.f, 0.f))
        return pandora::STATUS_CODE_FAILURE;

      const CaloHit *pBestCaloHit = NULL;
      float bestOrderParameter = std::numeric_limits<float>::max();
      pandora::CaloHitList deleteConnectionCaloHitList;

      // find the best connector with the smallest order parameter
      for(ConnectorList::const_iterator connectorIter = backwardConnectorList.begin(), connectorEndIter = backwardConnectorList.end() ;
          connectorEndIter != connectorIter ; ++connectorIter)
      {
        const Connector *pConnector = *connectorIter;
        const CaloHit *pFromCaloHit = pConnector->GetFrom();
        const float orderParameter(this->GetOrderParameter(pConnector, referenceVector));

        if(orderParameter< bestOrderParameter)
        {
          if(NULL != pBestCaloHit)
            deleteConnectionCaloHitList.push_back(pBestCaloHit);

          bestOrderParameter = orderParameter;
          pBestCaloHit = pFromCaloHit;
        }
        else
        {
          deleteConnectionCaloHitList.push_back(pFromCaloHit);
        }
      }

      if(NULL != pBestCaloHit)
        caloHitCleaningMap[pCaloHit] = deleteConnectionCaloHitList;
    }

    // remove all un-needed connections
    for(CaloHitCleaningMap::const_iterator iter = caloHitCleaningMap.begin(), endIter = caloHitCleaningMap.end() ;
        endIter != iter ; ++iter)
    {
      for(pandora::CaloHitList::const_iterator hitIter = iter->second.begin(), hitEndIter = iter->second.end() ;
          hitEndIter != hitIter ; ++hitIter)
      {
        const arbor_content::CaloHit *const pCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(*hitIter);
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::RemoveConnectionBetween(iter->first, pCaloHit));
      }
    }

    caloHitCleaningMap.clear();

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ConnectorCleaningTool::CleanCaloHits(const pandora::OrderedCaloHitList &orderedCaloHitList) const
  {
    if(1 == m_strategy)
    {
      for(pandora::OrderedCaloHitList::const_iterator iter = orderedCaloHitList.begin(), endIter = orderedCaloHitList.end() ;
          endIter != iter ; ++iter)
      {
        if(iter->second->empty())
          continue;

        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->CleanCaloHits(iter->second));
      }
    }
    else
    {
      for(pandora::OrderedCaloHitList::const_reverse_iterator iter = orderedCaloHitList.rbegin(), endIter = orderedCaloHitList.rend() ;
          endIter != iter ; ++iter)
      {
        if(iter->second->empty())
          continue;

        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->CleanCaloHits(iter->second));
      }
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ConnectorCleaningTool::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_backwardConnectorWeight = 1.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "BackwardConnectorWeight", m_backwardConnectorWeight));

    m_forwardConnectorWeight = 3.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ForwardConnectorWeight", m_forwardConnectorWeight));

    m_backwardReferenceDirectionDepth = 1;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "BackwardReferenceDirectionDepth", m_backwardReferenceDirectionDepth));

    m_forwardReferenceDirectionDepth = 2;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ForwardReferenceDirectionDepth", m_forwardReferenceDirectionDepth));

    if(m_backwardReferenceDirectionDepth == 0 || m_forwardReferenceDirectionDepth == 0)
      return pandora::STATUS_CODE_INVALID_PARAMETER;

    m_orderParameterAnglePower = 1.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "OrderParameterAnglePower", m_orderParameterAnglePower));

    m_orderParameterDistancePower = 5.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "OrderParameterDistancePower", m_orderParameterDistancePower));

    m_strategy = 2;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "Strategy", m_strategy));

    if(2 < m_strategy)
      return pandora::STATUS_CODE_INVALID_PARAMETER;

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

