  /// \file ArborContentApi.cc
/*
 *
 * ArborContentApi.cc source template automatically generated by a class generator
 * Creation date : mar. avr. 28 2015
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


#include "ArborApi/ArborContentApi.h"

#include "ArborObjects/CaloHit.h"
#include "ArborObjects/Connector.h"
#include "ArborObjects/ArborMetaData.h"


//------------------------------------------------------------------------------------------------------------------------------------------

bool ArborContentApi::IsConnected(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2)
{
	return pCaloHit1->m_pCaloHitMetaData->IsConnected(pCaloHit2);
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool ArborContentApi::IsConnected(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2, arbor_content::ConnectorDirection direction)
{
	return pCaloHit1->m_pCaloHitMetaData->IsConnected(pCaloHit2, direction);
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::FindConnector(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2, const arbor_content::Connector *&pConnector)
{
	return pCaloHit1->m_pCaloHitMetaData->FindConnector(pCaloHit2, pConnector);
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::FindConnector(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2, arbor_content::ConnectorDirection direction,
		const arbor_content::Connector *&pConnector)
{
	return pCaloHit1->m_pCaloHitMetaData->FindConnector(pCaloHit2, direction, pConnector);
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool ArborContentApi::HasAnyConnection(const arbor_content::CaloHit *const pCaloHit)
{
	return pCaloHit->m_pCaloHitMetaData->HasAnyConnection();
}

//------------------------------------------------------------------------------------------------------------------------------------------

const arbor_content::ConnectorList &ArborContentApi::GetConnectorList(const arbor_content::CaloHit *const pCaloHit)
{
	return pCaloHit->m_pCaloHitMetaData->GetConnectorList();
}

//------------------------------------------------------------------------------------------------------------------------------------------

const arbor_content::ConnectorList &ArborContentApi::GetConnectorList(const arbor_content::CaloHit *const pCaloHit, arbor_content::ConnectorDirection direction)
{
	return pCaloHit->m_pCaloHitMetaData->GetConnectorList(direction);
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool ArborContentApi::IsSeed(const arbor_content::CaloHit *const pCaloHit)
{
	return pCaloHit->m_pCaloHitMetaData->IsSeed();
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool ArborContentApi::IsLeaf(const arbor_content::CaloHit *const pCaloHit)
{
	return pCaloHit->m_pCaloHitMetaData->IsLeaf();
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::Connect(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2, arbor_content::ConnectorDirection direction,
		float referenceLength)
{
	const arbor_content::Connector *pConnector = NULL;
	return ArborContentApi::Connect(pCaloHit1, pCaloHit2, direction, pConnector, referenceLength);
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::Connect(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2, arbor_content::ConnectorDirection direction,
			const arbor_content::Connector *&pConnector, float referenceLength)
{
	pConnector = NULL;

	arbor_content::CaloHit *const pCaloHitFrom = direction == arbor_content::BACKWARD_DIRECTION ? ArborContentApi::Modifiable(pCaloHit2) : ArborContentApi::Modifiable(pCaloHit1);
	arbor_content::CaloHit *const pCaloHitTo = direction == arbor_content::BACKWARD_DIRECTION ? ArborContentApi::Modifiable(pCaloHit1) : ArborContentApi::Modifiable(pCaloHit2);

	pConnector = new arbor_content::Connector(pCaloHitFrom, pCaloHitTo, referenceLength);

	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pCaloHitFrom->m_pCaloHitMetaData->AddConnector(pConnector, arbor_content::FORWARD_DIRECTION));
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pCaloHitTo->m_pCaloHitMetaData->AddConnector(pConnector, arbor_content::BACKWARD_DIRECTION));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::RemoveConnectionBetween(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2)
{
	if(NULL == pCaloHit1 || NULL == pCaloHit2)
		return pandora::STATUS_CODE_INVALID_PARAMETER;

	const arbor_content::Connector *pConnector = NULL;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pCaloHit1->m_pCaloHitMetaData->FindConnector(pCaloHit2, pConnector));

	return ArborContentApi::RemoveAndDeleteConnector(pConnector);
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::RemoveConnector(const arbor_content::Connector *const pConnector)
{
	if(NULL == pConnector)
		return pandora::STATUS_CODE_INVALID_PARAMETER;

	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Modifiable(pConnector->GetFrom())->m_pCaloHitMetaData->RemoveConnector(pConnector));
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Modifiable(pConnector->GetTo())->m_pCaloHitMetaData->RemoveConnector(pConnector));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::RemoveAndDeleteConnector(const arbor_content::Connector *const pConnector)
{
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::RemoveConnector(pConnector));
	delete ArborContentApi::Modifiable(pConnector);

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::RemoveAndDeleteAllConnections(const arbor_content::CaloHit *const pCaloHit)
{
	arbor_content::ConnectorList backwardConnectorList(pCaloHit->m_pCaloHitMetaData->GetConnectorList(arbor_content::BACKWARD_DIRECTION));
	arbor_content::ConnectorList forwardConnectorList(pCaloHit->m_pCaloHitMetaData->GetConnectorList(arbor_content::FORWARD_DIRECTION));

	for(arbor_content::ConnectorList::const_iterator iter = backwardConnectorList.begin(), endIter = backwardConnectorList.end() ;
			endIter != iter ; ++iter)
	{
		const arbor_content::Connector *const pConnector = *iter;
		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::RemoveAndDeleteConnector(pConnector));
	}

	for(arbor_content::ConnectorList::const_iterator iter = forwardConnectorList.begin(), endIter = forwardConnectorList.end() ;
			endIter != iter ; ++iter)
	{
		const arbor_content::Connector *const pConnector = *iter;
		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::RemoveAndDeleteConnector(pConnector));
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::Tag(const arbor_content::CaloHit *const pCaloHit, arbor_content::HitTag tag, bool value)
{
	ArborContentApi::Modifiable(pCaloHit)->m_hitTagMap.set(tag, value);
	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool ArborContentApi::IsTagged(const arbor_content::CaloHit *const pCaloHit, arbor_content::HitTag tag)
{
	return pCaloHit->m_hitTagMap.test(tag);
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::ResetTags(const arbor_content::CaloHit *const pCaloHit)
{
	if(NULL == pCaloHit)
		return pandora::STATUS_CODE_INVALID_PARAMETER;

	ArborContentApi::Modifiable(pCaloHit)->m_hitTagMap.reset();

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::InitializeReclustering(const pandora::Algorithm &algorithm, const pandora::TrackList &inputTrackList,
		const pandora::ClusterList &inputClusterList, std::string &originalClustersListName)
{
	// Initialize re-clustering within pandora
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::InitializeReclustering(algorithm, inputTrackList, inputClusterList, originalClustersListName));

	// Current calo hit list here is the one for re-clustering
	// Get it, save and create new meta data for each
	const pandora::CaloHitList *pCaloHitList = NULL;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(algorithm, pCaloHitList));

	for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), endIter = pCaloHitList->end() ;
			endIter != iter ; ++iter)
	{
		const arbor_content::CaloHit *const pCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(*iter);

		if(NULL == pCaloHit)
			return pandora::STATUS_CODE_FAILURE;

		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Modifiable(pCaloHit)->SaveReclusterMetaData(originalClustersListName));
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::RunReclusteringAlgorithm(const pandora::Algorithm &algorithm, const std::string &clusteringAlgorithmName,
		const pandora::ClusterList *&pNewClusterList, std::string &newClusterListName)
{
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunClusteringAlgorithm(algorithm, clusteringAlgorithmName, pNewClusterList, newClusterListName));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::PostRunReclusteringAlgorithm(const pandora::Algorithm &algorithm, const std::string &clusterListName)
{
	// Get the current re-clustering calo hit list
	// Save meta data for each calo hit
	const pandora::CaloHitList *pCaloHitList = NULL;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(algorithm, pCaloHitList));

	for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), endIter = pCaloHitList->end() ;
			endIter != iter ; ++iter)
	{
		const arbor_content::CaloHit *const pCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(*iter);

		if(NULL == pCaloHit)
			return pandora::STATUS_CODE_FAILURE;

		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Modifiable(pCaloHit)->SaveReclusterMetaData(clusterListName));
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContentApi::EndReclustering(const pandora::Algorithm &algorithm, const std::string &selectedClusterListName)
{
	const pandora::CaloHitList *pCaloHitList = NULL;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(algorithm, pCaloHitList));

	for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), endIter = pCaloHitList->end() ;
			endIter != iter ; ++iter)
	{
		const arbor_content::CaloHit *const pCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(*iter);

		if(NULL == pCaloHit)
			return pandora::STATUS_CODE_FAILURE;

		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Modifiable(pCaloHit)->EndReclustering(selectedClusterListName));
	}

	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::EndReclustering(algorithm, selectedClusterListName));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

template <typename T>
T *ArborContentApi::Modifiable(const T *const pT)
{
	return const_cast<T*>(pT);
}

//------------------------------------------------------------------------------------------------------------------------------------------


template arbor_content::CaloHit   *ArborContentApi::Modifiable(const arbor_content::CaloHit   *const);
template arbor_content::Connector *ArborContentApi::Modifiable(const arbor_content::Connector *const);

