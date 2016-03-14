  /// \file NeutralVicinityReclusteringAlgorithm.h
/*
 *
 * NeutralVicinityReclusteringAlgorithm.h header template automatically generated by a class generator
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


#ifndef NEUTRALVICINITYRECLUSTERINGALGORITHM_H
#define NEUTRALVICINITYRECLUSTERINGALGORITHM_H

#include "Pandora/Algorithm.h"
#include "Pandora/PandoraInternal.h"

namespace arbor_content
{

/** 
 *  @brief  NeutralVicinityReclusteringAlgorithm class
 */ 
class NeutralVicinityReclusteringAlgorithm : public pandora::Algorithm
{
public:
	/**
	 *  @brief  Factory class for instantiating algorithm
	 */
	class Factory : public pandora::AlgorithmFactory
	{
	public:
		pandora::Algorithm *CreateAlgorithm() const;
	};

private:
	pandora::StatusCode Run();
	pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

private:
	float                  m_minChi2ToRunReclustering;
	float                  m_maxChi2ToStopReclustering;
	pandora::StringVector  m_clusteringAlgorithmList;
	std::string            m_associationAlgorithmName;
	std::string            m_trackClusterAssociationAlgName;
	std::string            m_monitoringAlgorithmName;
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *NeutralVicinityReclusteringAlgorithm::Factory::CreateAlgorithm() const
{
    return new NeutralVicinityReclusteringAlgorithm();
}

} 

#endif  //  NEUTRALVICINITYRECLUSTERINGALGORITHM_H
