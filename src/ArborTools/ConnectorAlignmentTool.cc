  /// \file ConnectorAlignmentTool.cc
/*
 *
 * ConnectorAlignmentTool.cc source template automatically generated by a class generator
 * Creation date : lun. juin 27 2016
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


#include "ArborTools/ConnectorAlignmentTool.h"

#include "Pandora/AlgorithmHeaders.h"

#include "ArborObjects/CaloHit.h"
#include "ArborObjects/Connector.h"
#include "ArborHelpers/CaloHitHelper.h"
#include "ArborHelpers/GeometryHelper.h"

namespace arbor_content
{

pandora::StatusCode ConnectorAlignmentTool::Process(const pandora::Algorithm &algorithm, const pandora::CaloHitList *const pCaloHitList)
{
	if(pCaloHitList->empty())
		return pandora::STATUS_CODE_SUCCESS;

	pandora::OrderedCaloHitList orderedCaloHitList;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetOrderedCaloHitList(algorithm, pCaloHitList, orderedCaloHitList));

	ConnectorList connectorList;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetConnectorList(orderedCaloHitList, connectorList));

	for(ConnectorList::const_iterator connectorIter = connectorList.begin(), connectorEndIter = connectorList.end() ;
			connectorEndIter != connectorIter ; ++connectorIter)
	{
		const Connector *const pConnector = *connectorIter;

		const bool isFromAvailable(PandoraContentApi::IsAvailable<pandora::CaloHit>(algorithm, pConnector->GetFrom()));
		const bool isToAvailable(PandoraContentApi::IsAvailable<pandora::CaloHit>(algorithm, pConnector->GetTo()));

		// check for availability
		if( (m_connectOnlyAvailable) && (!isFromAvailable || !isToAvailable) )
			continue;

		// connect in forward and backward directions
		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->ConnectForward(orderedCaloHitList, pConnector));
		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->ConnectBackward(orderedCaloHitList, pConnector));
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ConnectorAlignmentTool::GetOrderedCaloHitList(const pandora::Algorithm &algorithm, const pandora::CaloHitList *const pCaloHitList, pandora::OrderedCaloHitList &orderedCaloHitList) const
{
	// need filter ?
	if(m_connectOnlyAvailable || !m_shouldUseIsolatedHits)
	{
		for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), endIter = pCaloHitList->end() ;
				endIter != iter ; ++iter)
		{
			if(m_connectOnlyAvailable && !PandoraContentApi::IsAvailable(algorithm, *iter))
				continue;

			if(!m_shouldUseIsolatedHits && (*iter)->IsIsolated())
				continue;

			PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, orderedCaloHitList.Add(*iter));
		}
	}
	else
	{
		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, orderedCaloHitList.Add(*pCaloHitList));
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ConnectorAlignmentTool::GetConnectorList(const pandora::OrderedCaloHitList &orderedCaloHitList, ConnectorList &connectorList) const
{
	for(pandora::OrderedCaloHitList::const_iterator layerIter = orderedCaloHitList.begin(), layerEndIter = orderedCaloHitList.end() ;
			layerEndIter != layerIter ; ++layerIter)
	{
		for(pandora::CaloHitList::const_iterator iter = layerIter->second->begin(), endIter = layerIter->second->end() ;
				endIter != iter ; ++iter)
		{
			const CaloHit *pCaloHit = dynamic_cast<const CaloHit *>(*iter);

			if(NULL == pCaloHit)
				return pandora::STATUS_CODE_FAILURE;

			const ConnectorList &backwardConnectorList(ArborContentApi::GetConnectorList(pCaloHit, BACKWARD_DIRECTION));

			if(!backwardConnectorList.empty())
				connectorList.insert(backwardConnectorList.begin(), backwardConnectorList.end());
		}
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ConnectorAlignmentTool::ConnectBackward(const pandora::OrderedCaloHitList &orderedCaloHitList, const Connector *const pConnector) const
{
	const pandora::CartesianVector &connectorVector(pConnector->GetVector(BACKWARD_DIRECTION));

	const pandora::Granularity fromGranularity(this->GetPandora().GetGeometry()->GetHitTypeGranularity(pConnector->GetFrom()->GetHitType()));
	const float maxConnectionAngleFrom(fromGranularity <= pandora::FINE ? m_maxConnectionAngleFine : m_maxConnectionAngleCoarse);
	const float maxConnectionDistanceFrom(fromGranularity <= pandora::FINE ? m_maxConnectionDistanceFine : m_maxConnectionDistanceCoarse);

	if(0 == pConnector->GetFrom()->GetPseudoLayer())
		return pandora::STATUS_CODE_SUCCESS;

	const unsigned int startFromPseudoLayer(pConnector->GetFrom()->GetPseudoLayer()-1);
	const unsigned int endFromPseudoLayer(pConnector->GetFrom()->GetPseudoLayer() <= m_maxPseudoLayerConnection+1 ? 0 : pConnector->GetFrom()->GetPseudoLayer()-(m_maxPseudoLayerConnection+1));

	for(unsigned int pl = startFromPseudoLayer ; pl != endFromPseudoLayer ; --pl)
	{
		pandora::OrderedCaloHitList::const_iterator findIter = orderedCaloHitList.find(pl);

		if(orderedCaloHitList.end() == findIter)
			continue;

		for(pandora::CaloHitList::const_iterator iter = findIter->second->begin(), endIter = findIter->second->end() ;
				endIter != iter ; ++iter)
		{
			const CaloHit *const pCaloHit = dynamic_cast<const CaloHit *const>(*iter);

			if(NULL == pCaloHit)
				continue;

			// check types
			if(m_shouldConnectOnlySameHitType && pConnector->GetFrom()->GetHitType() != pCaloHit->GetHitType())
				continue;

			const pandora::CartesianVector differenceVector(pCaloHit->GetPositionVector() - pConnector->GetFrom()->GetPositionVector());

			if(maxConnectionDistanceFrom < differenceVector.GetMagnitude())
				continue;

			if(maxConnectionAngleFrom > connectorVector.GetOpeningAngle(differenceVector))
				continue;

			// check if already connected
			if(ArborContentApi::IsConnected(pConnector->GetFrom(), pCaloHit, BACKWARD_DIRECTION))
				continue;

			PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Connect(pConnector->GetFrom(), pCaloHit, BACKWARD_DIRECTION));
		}
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ConnectorAlignmentTool::ConnectForward(const pandora::OrderedCaloHitList &orderedCaloHitList, const Connector *const pConnector) const
{
	const pandora::CartesianVector &connectorVector(pConnector->GetVector(FORWARD_DIRECTION));

	const pandora::Granularity toGranularity(this->GetPandora().GetGeometry()->GetHitTypeGranularity(pConnector->GetTo()->GetHitType()));
	const float maxConnectionAngleTo(toGranularity <= pandora::FINE ? m_maxConnectionAngleFine : m_maxConnectionAngleCoarse);
	const float maxConnectionDistanceTo(toGranularity <= pandora::FINE ? m_maxConnectionDistanceFine : m_maxConnectionDistanceCoarse);

	const unsigned int startToPseudoLayer(pConnector->GetTo()->GetPseudoLayer()+1);
	const unsigned int endToPseudoLayer(pConnector->GetTo()->GetPseudoLayer()+m_maxPseudoLayerConnection+1);

	for(unsigned int pl = startToPseudoLayer ; pl <= endToPseudoLayer ; ++pl)
	{
		pandora::OrderedCaloHitList::const_iterator findIter = orderedCaloHitList.find(pl);

		if(orderedCaloHitList.end() == findIter)
			continue;

		for(pandora::CaloHitList::const_iterator iter = findIter->second->begin(), endIter = findIter->second->end() ;
				endIter != iter ; ++iter)
		{
			const CaloHit *const pCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(*iter);

			if(NULL == pCaloHit)
				continue;

			// check types
			if(m_shouldConnectOnlySameHitType && pConnector->GetTo()->GetHitType() != pCaloHit->GetHitType())
				continue;

			const pandora::CartesianVector differenceVector(pCaloHit->GetPositionVector() - pConnector->GetTo()->GetPositionVector());

			if(maxConnectionDistanceTo < differenceVector.GetMagnitude())
				continue;

			if(maxConnectionAngleTo > connectorVector.GetOpeningAngle(differenceVector))
				continue;

			// check if already connected
			if(ArborContentApi::IsConnected(pConnector->GetTo(), pCaloHit, FORWARD_DIRECTION))
				continue;

			PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Connect(pConnector->GetTo(), pCaloHit, FORWARD_DIRECTION));
		}
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ConnectorAlignmentTool::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
	m_connectOnlyAvailable = true;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ConnectOnlyAvailable", m_connectOnlyAvailable));

	m_shouldUseIsolatedHits = false;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ShouldUseIsolatedHits", m_shouldUseIsolatedHits));

	m_shouldConnectOnlySameHitType = true;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ShouldConnectOnlySameHitType", m_shouldConnectOnlySameHitType));

	m_maxPseudoLayerConnection = 2;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"MaxPseudoLayerConnection", m_maxPseudoLayerConnection));

	m_maxConnectionAngleFine = M_PI/4.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"MaxConnectionAngleFine", m_maxConnectionAngleFine));

	m_maxConnectionAngleCoarse = M_PI/4.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"MaxConnectionAngleCoarse", m_maxConnectionAngleCoarse));

	m_maxConnectionDistanceFine = 30.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"MaxConnectionDistanceFine", m_maxConnectionDistanceFine));

	m_maxConnectionDistanceCoarse = 65.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"MaxConnectionDistanceCoarse", m_maxConnectionDistanceCoarse));

	return pandora::STATUS_CODE_SUCCESS;
}

} 

