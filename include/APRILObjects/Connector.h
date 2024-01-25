/// \file Connector.h
/*
 *
 * Connector.h header template automatically generated by a class generator
 * Creation date : ven. mars 20 2015
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


#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "Pandora/StatusCodes.h"
#include "Objects/CartesianVector.h"

#include "APRILApi/APRILInputTypes.h"
#include "APRILApi/APRILContentApi.h"

namespace april_content
{

/** 
 *  @brief  Connector class
 */
class Connector
{
public:
	/**
	 *  @brief  Get the 'from' calo hit
	 */
	const april_content::CaloHit *GetFrom() const;

	/**
	 *  @brief  Get the 'to' calo hit
	 */
	const april_content::CaloHit *GetTo() const;

	/**
	 *  @brief  Get the 'to' or 'from' calo hit (from (to) for BACKWARD_DIRECTION (FORWARD_DIRECTION))
	 */
	const april_content::CaloHit *Get(ConnectorDirection direction) const;

	/**
	 *  @brief  Get the connector length
	 */
	float GetLength() const;

	/**
	 *  @brief  Get the reference length
	 */
	float GetReferenceLength() const;

	/**
	 *  @brief  Get the normalized length
	 */
	float GetNormalizedLength() const;

	/**
	 *  @brief  Get the connector vector depending on the asked direction
	 */
	pandora::CartesianVector GetVector(ConnectorDirection direction = FORWARD_DIRECTION) const;

	/**
	 *  @brief  Whether the calo hit is the 'from' calo hit of this connector
	 */
	bool IsFrom(const april_content::CaloHit *const pCaloHit) const;

	/**
	 *  @brief  Whether the calo hit is the 'to' calo hit of this connector
	 */
	bool IsTo(const april_content::CaloHit *const pCaloHit) const;

	/**
	 *  @brief  Get the opening angle with an other connector (radian)
	 */
	float GetOpeningAngle(const Connector *const pConnector) const;

	unsigned int GetCreationStage() const;

private:
	/**
	 *  @brief  Constructor with two calo hits and a reference length
	 */
	Connector(const april_content::CaloHit *const pFromCaloHit, const april_content::CaloHit *const pToCaloHit,
        float referenceLength = 1.f, unsigned int creationStage = -1);

	/**
	 *  @brief  Destructor
	 */
	~Connector();

private:
	const CaloHit                        *m_pFromCaloHit;     ///< The 'from' calo hit of this connector
	const CaloHit                        *m_pToCaloHit;       ///< The 'to' calo hit of this connector
	float                                 m_referenceLength;  ///< The reference length on connector creation
        unsigned int                          m_creationStage;    ///< The clustering stage of connector creation

	// friendship
	friend class CaloHit;
	friend class ::APRILContentApi;
};

} 

#endif  //  CONNECTOR_H
