  /// \file ArborContentApi.h
/*
 *
 * ArborContentApi.h header template automatically generated by a class generator
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


#ifndef ARBORCONTENTAPI_H
#define ARBORCONTENTAPI_H

#include "Api/PandoraContentApi.h"
#include "ArborApi/ArborInputTypes.h"

namespace arbor_content { class CaloHit; class Connector; }

/** 
 *  @brief  ArborContentApi class
 */ 
class ArborContentApi 
{
public:
	// Connector related Api

	/**
	 *  @brief  Whether the two calo hits are connected (backward or forward directions)
	 *
	 *  @param  pCaloHit1 the first calo hit
	 *  @param  pCaloHit2 the second calo hit
	 *
	 *  @return  boolean
	 */
	static bool IsConnected(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2);

	/**
	 *  @brief  Whether the two calo hits are connected in the given direction
	 *
	 *  @param  pCaloHit1 the first calo hit
	 *  @param  pCaloHit2 the second calo hit
	 *  @param  direction the direction in which the calo hits may be connected
	 *
	 *  @return boolean
	 */
	static bool IsConnected(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2, arbor_content::ConnectorDirection direction);

	/**
	 *  @brief  Find a connector that links the two calo hits
	 *
	 *  @param  pCaloHit1 the first calo hit
	 *  @param  pCaloHit2 the second calo hit
	 *  @param  pConnector the connector address to receive
	 */
	static pandora::StatusCode FindConnector(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2, const arbor_content::Connector *&pConnector);

	/**
	 *  @brief  Find a connector that links the two calo hits in a given direction
	 *
	 *  @param  pCaloHit1 the first calo hit
	 *  @param  pCaloHit2 the second calo hit
	 *  @param  direction the direction in which the two calo hits are connected
	 *  @param  pConnector the connector address to receive
	 */
	static pandora::StatusCode FindConnector(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2, arbor_content::ConnectorDirection direction,
			const arbor_content::Connector *&pConnector);

	/**
	 *  @brief  Whether the calo hit has at least one connection
	 *
	 *  @param  pCaloHit the calo hit to test
	 *
	 *  @return  boolean
	 */
	static bool HasAnyConnection(const arbor_content::CaloHit *const pCaloHit);

	/**
	 *  @brief  Get the global connector list (backward and forward) of the calo hit
	 *
	 *  @param  pCaloHit the calo hit to get the connector list
	 *
	 *  @return  ConnectorList the connector list (std::set<const Connector*>)
	 */
	static const arbor_content::ConnectorList &GetConnectorList(const arbor_content::CaloHit *const pCaloHit);

	/**
	 *  @brief  Get the connector list of a calo hit for a given direction
	 *
	 *  @param  pCaloHit the calo hit to get the connector list
	 *  @param  direction the direction of the connector list
	 *
	 *  @return  ConnectorList the connector list (std::set<const Connector*>)
	 */
	static const arbor_content::ConnectorList &GetConnectorList(const arbor_content::CaloHit *const pCaloHit, arbor_content::ConnectorDirection direction);

	/**
	 *  @brief  Whether the calo hit is a seed calo hit in the tree.
	 *
	 *  @param  pCaloHit the calo hit to test
	 *
	 *  @return  boolean
	 */
	static bool IsSeed(const arbor_content::CaloHit *const pCaloHit);

	/**
	 *  @brief  Whether the calo hit is a leaf calo hit in the tree
	 *
	 *  @param  pCaloHit the calo hit to test
	 *
	 *  @return  boolean
	 */
	static bool IsLeaf(const arbor_content::CaloHit *const pCaloHit);

	/**
	 *  @brief  Connect the two calo hits in a given direction
	 *
	 *  @param  pCaloHit1 the first calo hit to connect
	 *  @param  pCaloHit2 the second calo hit to connect
	 *  @param  direction the direction of the connection (FORWARD_DIRECTION means that pCaloHit1 will be the "from")
	 *  @param  referenceLength the reference length assigned to the connector
	 */
	static pandora::StatusCode Connect(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2,
			arbor_content::ConnectorDirection direction, float referenceLength = 1.f);

	/**
	 *  @brief  Connect the two calo hits in a given direction
	 *
	 *  @param  pCaloHit1 the first calo hit to connect
	 *  @param  pCaloHit2 the second calo hit to connect
	 *  @param  direction the direction of the connection (FORWARD_DIRECTION means that pCaloHit1 will be the "from")
	 *  @param  pConnector the connector address to receive
	 *  @param  referenceLength the reference length assigned to the connector
	 */
	static pandora::StatusCode Connect(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2,
			arbor_content::ConnectorDirection direction, const arbor_content::Connector *&pConnector, float referenceLength = 1.f);

	/**
	 *  @brief  Remove and delete the connection between the two calo hits.
	 *
	 *  @param  pCaloHit1 the first calo hit to disconnect
	 *  @param  pCaloHit2 the second calo hit to disconnect
	 */
	static pandora::StatusCode RemoveConnectionBetween(const arbor_content::CaloHit *const pCaloHit1, const arbor_content::CaloHit *const pCaloHit2);

	/**
	 *  @brief  Remove the connection between the two calo hits and delete the connector
	 *
	 *  @param  pConnector the connector to remove and delete
	 */
	static pandora::StatusCode RemoveAndDeleteConnector(const arbor_content::Connector *const pConnector);

	/**
	 *  @brief  Remove and delete all the connections of the calo hit
	 *
	 *  @param  pCaloHit the calo hit to remove all connections
	 */
	static pandora::StatusCode RemoveAndDeleteAllConnections(const arbor_content::CaloHit *const pCaloHit);

	// calo hit related Api

	/**
	 *  @brief  Set the calo hit tag
	 *
	 *  @param  pCaloHit the calo hit to tag
	 *  @param  tag the reconstruction tag to apply
	 *  @param  value the tag value to assign
	 */
	static pandora::StatusCode Tag(const arbor_content::CaloHit *const pCaloHit, arbor_content::HitTag tag, bool value);

	/**
	 *  @brief  Whether the calo hit has been tagged
	 *
	 *  @param  pCaloHit the calo hit to test
	 *  @param  tag the reconstruction tag to test
	 *
	 *  @return  boolean
	 */
	static bool IsTagged(const arbor_content::CaloHit *const pCaloHit, arbor_content::HitTag tag);

	/**
	 *  @brief  Reset the calo hit tags
	 *
	 *  @param  pCaloHit the calo hit to reset tags
	 */
	static pandora::StatusCode ResetTags(const arbor_content::CaloHit *const pCaloHit);

	/**
	 *  @brief  Initialize Arbor re-clustering. Save calo hit meta data and create a new one for each calo hit
	 *
	 *  @param  algorithm the parent algorithm that initializes the re-clustering
	 *  @param  inputTrackList the list of tracks used for re-clustering
	 *  @param  inputClusterList the list of cluster used for re-clustering
	 *  @param  originalClustersListName the list of the original cluster to receive
	 */
	static pandora::StatusCode InitializeReclustering(const pandora::Algorithm &algorithm, const pandora::TrackList &inputTrackList,
			const pandora::ClusterList &inputClusterList, std::string &originalClustersListName);

	/**
	 *  @brief  Run a re-clustering algorithm. Must be called only to run a clustering algorithm in a re-clustering process
	 *
	 *  @param  algorithm the parent algorithm running the (re)clustering algorithm
	 *  @param  clusteringAlgorithmName the name of the clustering algorithm
	 *  @param  pNewClusterList the list of created clusters to receive
	 *  @param  newClusterListName the cluster list name to receive
	 */
	static pandora::StatusCode RunReclusteringAlgorithm(const pandora::Algorithm &algorithm, const std::string &clusteringAlgorithmName,
			const pandora::ClusterList *&pNewClusterList, std::string &newClusterListName);

	/**
	 *  @brief  Function to call just after a re-clustering algorithm and potential association algorithms
	 *
	 *  @param  algorithm the parent algorithm that run the re-clustering algorithm
	 *  @param  clusterListName the name of the cluster list
	 */
	static pandora::StatusCode PostRunReclusteringAlgorithm(const pandora::Algorithm &algorithm, const std::string &clusterListName);

	/**
	 *  @brief  End the re-clustering procedure.
	 *
	 *  @param  algorithm the parent algorithm that run the re-clustering
	 *  @param  selectedClusterListName the name of the selected cluster list
	 */
	static pandora::StatusCode EndReclustering(const pandora::Algorithm &algorithm, const std::string &selectedClusterListName);

private:
	/**
	 *  @brief  Remove the connection. Destructor not called, the connector responsibility is forwarded to the caller
	 *
	 *  @param  pConnector the connector to remove
	 */
	static pandora::StatusCode RemoveConnector(const arbor_content::Connector *const pConnector);

	/**
	 *  @brief  Alter the object (const_cast<T*>) for internal modification through the API
	 *
	 *  @param  pT the object to alter
	 *
	 *  @return T* the altered object
	 */
	template <typename T>
	static T *Modifiable(const T *const pT);
}; 


#endif  //  ARBORCONTENTAPI_H
