  /// \file ConnectorCleaningTool.h
/*
 *
 * ConnectorCleaningTool.h header template automatically generated by a class generator
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


#ifndef CONNECTORCLEANINGTOOL_H
#define CONNECTORCLEANINGTOOL_H

#include "ArborTools/ConnectorAlgorithmTool.h"

#include "Objects/CartesianVector.h"

namespace arbor_content
{

class CaloHit;
class Connector;

/** 
 *  @brief  ConnectorCleaningTool class
 */ 

class ConnectorOrderParameter
{
public:
	ConnectorOrderParameter()
	: m_distance(std::numeric_limits<float>::max()),
	  m_openingAngle(std::numeric_limits<float>::max()),
	  m_nConnectons(0),
	  m_creationStage(-1),
	  m_fromHitPos(0., 0., 0.),
	  m_orderParameter(std::numeric_limits<float>::max())
	{
		//std::cout << "best ConnectorOrderParameter: " << m_distance << ", " << m_openingAngle << ", " << m_orderParameter << std::endl;
		//std::cout << " --- small angle range: " << m_smallAngleRange << std::endl;
	}

	ConnectorOrderParameter(float distance, float openingAngle, unsigned int nConnections, unsigned int creationStage, pandora::CartesianVector formHitPos) 
	: m_distance(distance), 
	  m_openingAngle(openingAngle),
	  m_nConnectons(nConnections),
	  m_creationStage(creationStage),
	  m_fromHitPos(formHitPos)
	{
        m_orderParameter = std::pow(m_openingAngle, m_orderParameterAnglePower) * 
			               std::pow(m_distance, m_orderParameterDistancePower);
	}

	bool operator<(const ConnectorOrderParameter& a) const
	{
		//std::cout << " ---+++ small angle range: " << m_smallAngleRange << std::endl;

		if( m_creationStage != a.m_creationStage )
		{
			return m_creationStage < a.m_creationStage;
		}

		if(m_openingAngle < m_smallAngleRange && a.m_openingAngle < m_smallAngleRange && m_distance != a.m_distance)
		{
			//std::cout << "small angle, distance: " << m_distance << ", " << a.m_distance << std::endl;
			return m_distance < a.m_distance;
		}

		if(m_openingAngle == a.m_openingAngle && m_distance == a.m_distance && m_nConnectons != a.m_nConnectons)
		{
			// sure, hit with more connectors has small (good) order ...
			return m_nConnectons > a.m_nConnectons;
		}

		// if all parameters but position are the same, just take position
		if(m_nConnectons == a.m_nConnectons)
		{
			if(!(m_fromHitPos == a.m_fromHitPos))
			{
			    return ( m_fromHitPos.GetX() < a.m_fromHitPos.GetX() || 
			    		 m_fromHitPos.GetY() < a.m_fromHitPos.GetY() ||
			    	     m_fromHitPos.GetZ() < a.m_fromHitPos.GetZ() );
			}
			else
			{
				std::cout << "Connector are completely the same." << std::endl;
				throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);
			}
		}

		return m_orderParameter < a.m_orderParameter;
	}

	float                     m_distance;
	float                     m_openingAngle;
	unsigned int              m_nConnectons;
	unsigned int              m_creationStage;
	pandora::CartesianVector  m_fromHitPos;
	float                     m_orderParameter;

	static float              m_smallAngleRange;
	static float              m_orderParameterAnglePower;
	static float              m_orderParameterDistancePower;
};

class ConnectorCleaningTool : public ConnectorAlgorithmTool
{
	typedef std::map<const arbor_content::CaloHit*, pandora::CaloHitList> CaloHitCleaningMap;

public:
    /**
     *  @brief  Factory class for instantiating algorithm tool
     */
    class Factory : public pandora::AlgorithmToolFactory
    {
    public:
        pandora::AlgorithmTool *CreateAlgorithmTool() const;
    };

	/**
	 *  @brief  Destructor
	 */
	~ConnectorCleaningTool();

	/**
	 *  @brief  Clean the connectors of the calo hit list
	 *  @param  algorithm the parent algorithm to access pandora content
	 *
	 *  @param  pCaloHitList the calo hit list to process
	 */
	pandora::StatusCode Process(const pandora::Algorithm &algorithm, const pandora::CaloHitList *const pCaloHitList = nullptr);

	/**
	 *  @brief  Read settings from the xml handle
	 *
	 *  @param  xmlHandle the xml handle to settings
	 */
	pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

private:
	/**
	 *  @brief  Get the order parameter of the connector in the backward direction
	 *
	 *  @param  pConnector the connector to compute the order parameter
	 *  @param  referenceDirection the reference direction to compare with the connector
	 *
	 *  @return the order parameter
	 */
	//float GetOrderParameter(const arbor_content::Connector *const pConnector, const pandora::CartesianVector &referenceDirection) const;

	/**
	 *  @brief  Get the reference vector of the calo hit. The vector is oriented backwardly.
	 *
	 *  @param  pCaloHit the calo hit to evaluate the reference vector
	 *
	 *  @return the reference vector
	 */
	pandora::CartesianVector GetReferenceVector(const arbor_content::CaloHit *const pCaloHit) const;

	/**
	 *  @brief  Clean the connectors of the calo hits, ending with a tree structure
	 *
	 *  @param  pCaloHitList the calo hit list to clean
	 */
	pandora::StatusCode CleanCaloHits(const pandora::CaloHitList *const pCaloHitList) const;

	/**
	 *  @brief  Clean the connectors of the ordered calo hit list, ending with a tree structure
	 *
	 *  @param  orderedCaloHitList the ordered calo hit list to clean
	 */
	pandora::StatusCode CleanCaloHits(const pandora::OrderedCaloHitList &orderedCaloHitList) const;

private:
	unsigned int               m_strategy;
	float                      m_backwardConnectorWeight;
	float                      m_forwardConnectorWeight;
	unsigned int               m_backwardReferenceDirectionDepth;
	unsigned int               m_forwardReferenceDirectionDepth;
	float                      m_orderParameterAnglePower;
	float                      m_orderParameterDistancePower;
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::AlgorithmTool *ConnectorCleaningTool::Factory::CreateAlgorithmTool() const
{
	return new ConnectorCleaningTool();
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline ConnectorCleaningTool::~ConnectorCleaningTool()
{

}

} 

#endif  //  CONNECTORCLEANINGTOOL_H
