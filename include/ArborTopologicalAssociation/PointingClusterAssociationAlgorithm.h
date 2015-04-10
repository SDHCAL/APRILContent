  /// \file PointingClusterAssociationAlgorithm.h
/*
 *
 * PointingClusterAssociationAlgorithm.h header template automatically generated by a class generator
 * Creation date : jeu. avr. 9 2015
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


#ifndef POINTINGCLUSTERASSOCIATIONALGORITHM_H
#define POINTINGCLUSTERASSOCIATIONALGORITHM_H

#include "Pandora/Algorithm.h"


namespace arbor_content
{

/** 
 * @brief PointingClusterAssociationAlgorithm class
 */ 
class PointingClusterAssociationAlgorithm : public pandora::Algorithm
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

	/**
	 *  @brief  Whether the cluster is eligible for association
	 *
	 *  @param  pCluster address of a candidate cluster for association
	 *
	 *  @return  boolean
	 */
	bool CanMergeCluster(const pandora::Cluster *const pCluster) const;

	/**
	 *  @brief  Find the best parent cluster to merge a daughter one
	 *
	 *  @param  pDaughterCluster address of the daughter cluster
	 *  @param  clusterVector a cluster vector
	 *  @param  pBestParentCluster address of the best parent cluster to receive
	 */
	pandora::StatusCode FindBestParentCluster(const pandora::Cluster *const pDaughterCluster, const pandora::ClusterVector &clusterVector,
			const pandora::Cluster *&pBestParentCluster) const;

	/**
	 *
	 */
	pandora::StatusCode PerformBarycentreClusterComputation(const pandora::ClusterFitResult &daughterClusterFitResult, const pandora::CartesianVector &daughterClusterCentroid,
			const pandora::ClusterFitResult &parentClusterFitResult, const pandora::CartesianVector &parentClusterCentroid,
			const pandora::Cluster *const pParentCluster, const pandora::Cluster *&pBestBarycentreCluster, float &bestImpactParameter) const;

	/**
	 *
	 */
	pandora::StatusCode PerformInterceptClusterComputation(const pandora::ClusterFitResult &daughterClusterFitResult, const pandora::CartesianVector &daughterClusterCentroid,
			const pandora::ClusterFitResult &parentClusterFitResult, const pandora::CartesianVector &parentClusterCentroid,
			const pandora::Cluster *const pParentCluster, const pandora::Cluster *&pBestInterceptCluster, float &bestImpactParameter) const;

	/**
	 *
	 */
	pandora::StatusCode ChooseBestParentCluster(const pandora::Cluster *const pBarycentreParentCluster, const pandora::Cluster *const pInterceptParentCluster,
			const pandora::Cluster *const pDaughterCluster, const pandora::Cluster *&pBestParentCluster) const;

	pandora::StatusCode Run();
	pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

	unsigned int                    m_minNCaloHits;
	unsigned int                    m_maxNCaloHits;
	unsigned int                    m_minNPseudoLayers;
	unsigned int                    m_maxNPseudoLayers;
	float                            m_chi2AssociationCut;
	float                            m_clustersAngleCut;
	float                            m_barycentreImpactParameterCut;
	float                            m_interceptImpactParameterCut;
	float                            m_interceptClosestDistanceApproachCut;
}; 

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *PointingClusterAssociationAlgorithm::Factory::CreateAlgorithm() const
{
    return new PointingClusterAssociationAlgorithm();
}

} 

#endif  //  POINTINGCLUSTERASSOCIATIONALGORITHM_H