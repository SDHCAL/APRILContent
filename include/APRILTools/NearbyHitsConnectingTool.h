  /// \file NearbyHitsConnectingTool.h
/*
 *
 * NearbyHitsConnectingTool.h header template automatically generated by a class generator
 * Creation date : mar. d�c. 8 2015
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


#ifndef NEARBYHITSCONNECTINGTOOL_H
#define NEARBYHITSCONNECTINGTOOL_H

#include "APRILTools/ConnectorAlgorithmTool.h"

namespace april_content
{

class CaloHit;
class Connector;

/** 
 * @brief NearbyHitsConnectingTool class
 */ 
class NearbyHitsConnectingTool : public ConnectorAlgorithmTool 
{
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
	 *  @brief  Seed connectors from the calo hit list
	 *
	 *  @param  algorithm the parent algorithm to access pandora content
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
    unsigned int                m_metricType;
    float                       m_epsDBSCAN;
    unsigned int                m_minpDBSCAN;
	float                       m_maxTransverseDistanceFine;
	float                       m_maxTransverseDistanceCoarse;
	//unsigned int                m_maxPseudoLayerConnection;
	bool                        m_connectOnlyAvailable;
	bool                        m_shouldUseIsolatedHits;
	bool                        m_shouldDiscriminateConnectedHits;

};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::AlgorithmTool *NearbyHitsConnectingTool::Factory::CreateAlgorithmTool() const
{
	return new NearbyHitsConnectingTool();
}

} 

#endif  
