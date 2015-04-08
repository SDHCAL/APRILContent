  /// \file ArborMetaData.h
/*
 *
 * ArborMetaData.h header template automatically generated by a class generator
 * Creation date : sam. mars 21 2015
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


#ifndef ARBOR_META_DATA_H
#define ARBOR_META_DATA_H 1

#include "ArborApi/ArborInputTypes.h"
#include "ArborObjects/Connector.h"

namespace arbor_content
{

class CaloHit;
class Connector;

class CaloHitMetaData
{
private:
	/**
	 *  @brief  Constructor with the associated calo hit
	 */
	CaloHitMetaData(const arbor_content::CaloHit *const pCaloHit);

	/**
	 * @brief  Destructor
	 */
	~CaloHitMetaData();

	/**
	 *  @brief  Get the calo hit of this connector meta data
	 */
	const arbor_content::CaloHit *const GetCaloHit() const;

	/**
	 *  @brief  Whether the calo hit is connected with this one
	 */
	bool IsConnected(const arbor_content::CaloHit *const pCaloHit) const;

	/**
	 *  @brief  Whether the calo hit (contained in meta data) is connected with this one in the connector direction
	 */
	bool IsConnected(const arbor_content::CaloHit *const pCaloHit, ConnectorDirection direction)  const;

	/**
	 *  @brief  Find a connector.
	 */
	pandora::StatusCode FindConnector(const arbor_content::CaloHit *const pCaloHit, const Connector *&pConnector) const;

	/**
	 *  @brief  Find a connector in a given direction
	 */
	pandora::StatusCode FindConnector(const arbor_content::CaloHit *const pCaloHit, ConnectorDirection direction,
			const Connector *&pConnector) const;

	/**
	 *  @brief  Whether the pandora calo hit has at least one connection
	 */
	bool HasAnyConnection() const;

	/**
	 *  @brief  Get the global connector list (backward and forward)
	 */
	const ConnectorList &GetConnectorList() const;

	/**
	 *  @brief  Get the connector list for a given direction
	 */
	const ConnectorList &GetConnectorList(ConnectorDirection direction) const;

	/**
	 *  @brief  Whether the calo hit is a seed calo hit in the tree.
	 */
	bool IsSeed() const;

	/**
	 *  @brief  Whether the calo hit is a leaf calo hit in the tree
	 */
	bool IsLeaf() const;

	/**
	 *
	 */
	pandora::StatusCode AddConnector(const Connector *const pConnector, ConnectorDirection direction);

	/**
	 *  @brief  Remove the connection with an other calo hit
	 */
	pandora::StatusCode RemoveConnector(const Connector *const pConnector);

	/**
	 *  @brief  Remove all connections
	 */
	pandora::StatusCode RemoveAllConnections();

	const arbor_content::CaloHit *const     m_pCaloHit;

	ConnectorList                             m_allConnectorList;
	ConnectorList                             m_forwardConnectorList;
	ConnectorList                             m_backwardConnectorList;

	bool                                      m_isLeaf;
	bool                                      m_isSeed;

	friend class CaloHit;
};

} 

#endif  //  ARBOR_META_DATA_H