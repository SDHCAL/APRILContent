/// \file SurroundingHitsMergingAlgorithm.cc
/*
 *
 * SurroundingHitsMergingAlgorithm.cc source template automatically generated by a class generator
 * Creation date : jeu. nov. 19 2015
 *
 * This file is part of APRILContent libraries.
 * 
 * APRILContent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * APRILContent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with APRILContent.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#include "APRILTopologicalAssociation/SurroundingHitsMergingAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"

#include "APRILApi/APRILContentApi.h"
#include "APRILHelpers/ClusterHelper.h"
#include "APRILTools/CaloHitMergingTool.h"

namespace april_content
{

  pandora::StatusCode SurroundingHitsMergingAlgorithm::Run()
  {
    pandora::CaloHitList caloHitList; pandora::ClusterVector clusterVector;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetContents(caloHitList, clusterVector));

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, m_pCaloHitMergingTool->MergeCaloHits(*this, caloHitList, clusterVector));

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode SurroundingHitsMergingAlgorithm::GetAvailableCaloHitList(const pandora::CaloHitList *const pCaloHitList, pandora::CaloHitList &availableCaloHitList) const
  {
    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), endIter = pCaloHitList->end() ;
        endIter != iter ; ++iter)
    {
      const pandora::CaloHit *const pCaloHit(*iter);

      if(!PandoraContentApi::IsAvailable(*this, pCaloHit))
        continue;

      if(!m_shouldMergeIsolatedHits && pCaloHit->IsIsolated())
        continue;

      availableCaloHitList.push_back(pCaloHit);
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode SurroundingHitsMergingAlgorithm::GetContents(pandora::CaloHitList &caloHitList, pandora::ClusterVector &clusterVector) const
  {
    // get available calo hit list
    const pandora::CaloHitList *pCaloHitList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetAvailableCaloHitList(pCaloHitList, caloHitList));

    // get cluster list
	//
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    pandora::ClusterList clusterList(pClusterList->begin(), pClusterList->end());

	std::cout << "cluster: " << clusterList.size() << std::endl;

    for(pandora::StringVector::const_iterator iter = m_additionalClusterListNames.begin(), endIter = m_additionalClusterListNames.end() ;
        endIter != iter ; ++iter)
    {
      const pandora::ClusterList *pAdditionalClusterList = NULL;

      if(pandora::STATUS_CODE_SUCCESS != PandoraContentApi::GetList(*this, *iter, pAdditionalClusterList))
      {
        std::cout << "SurroundingHitsMergingAlgorithm: additional cluster list '" << *iter << "' is not available" << std::endl;
        continue;
      }

      clusterList.insert(clusterList.begin(), pAdditionalClusterList->begin(), pAdditionalClusterList->end());
    }

    clusterVector.insert(clusterVector.end(), clusterList.begin(), clusterList.end());

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode SurroundingHitsMergingAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_shouldMergeIsolatedHits = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldMergeIsolatedHits", m_shouldMergeIsolatedHits));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadVectorOfValues(xmlHandle,
        "AdditionalClusterListNames", m_additionalClusterListNames));

    m_pCaloHitMergingTool = NULL;
    pandora::AlgorithmTool *pAlgorithmTool = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ProcessAlgorithmTool(*this, xmlHandle,
        "CaloHitMerging", pAlgorithmTool));

    m_pCaloHitMergingTool = dynamic_cast<CaloHitMergingTool*>(pAlgorithmTool);

    if(NULL == m_pCaloHitMergingTool)
      return pandora::STATUS_CODE_INVALID_PARAMETER;

    return pandora::STATUS_CODE_SUCCESS;
  }

} 
