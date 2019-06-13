/// \file NearbyFragmentsMergingAlgorithm.cc
/*
 *
 * NearbyFragmentsMergingAlgorithm.cc source template automatically generated by a class generator
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

#include "Pandora/AlgorithmHeaders.h"
#include "PandoraMonitoringApi.h"
#include "PandoraMonitoring.h"

#include "ArborTopologicalAssociation/NearbyFragmentsMergingAlgorithm.h"

#include "ArborHelpers/SortingHelper.h"
#include "ArborHelpers/GeometryHelper.h"
#include "ArborHelpers/ReclusterHelper.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborHelpers/HistogramHelper.h"
#include "ArborHelpers/ClusterPropertiesHelper.h"

#include "ArborApi/ArborContentApi.h"

#include "ArborUtility/EventPreparationAlgorithm.h"

#include "ArborTools/TrackDrivenSeedingTool.h"
#include "ArborObjects/CaloHit.h"

#include <algorithm>

#define __DEBUG__ 0
#define __DEBUG1__ 0
#define __USEMCP__ 0

namespace arbor_content
{
  pandora::StatusCode NearbyFragmentsMergingAlgorithm::Run()
  {
	std::vector<ArborCluster*> clusterVector;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetEligibleClusters(clusterVector));

    // get fragments for merging
	m_clustersToMerge.clear();

	for(int i = 0; i < clusterVector.size(); ++i)
	{
		auto pCluster = clusterVector.at(i);
		

		const pandora::Cluster* const pandoraClu = dynamic_cast<const pandora::Cluster* const>(pCluster);
		bool isPhoton = PandoraContentApi::GetPlugins(*this)->GetParticleId()->IsPhoton(pandoraClu);

		if(isPhoton || pCluster->GetAssociatedTrackList().size() != 0) continue;

		pandora::CartesianVector centroid(0., 0., 0);
		ClusterHelper::GetCentroid(pCluster, centroid);
		pCluster->SetCentroid(centroid);

		/// help by MC truth
#if __USEMCP__
		try
		{
			isPhoton = pandora::MCParticleHelper::GetMainMCParticle(pandoraClu)->GetParticleId() == 22;
		}
		catch(pandora::StatusCodeException &)
		{
			std::cout << "MCP issue: " << pandoraClu << std::endl;
		}
#endif

		pCluster->SetPhoton(false);

#if __DEBUG__
		std::cout << " --- cluster : " << pCluster << ", energy: " << pCluster->GetHadronicEnergy() 
			      << ", COG: " << centroid.GetX() << ", " << centroid.GetY() << ", " << centroid.GetZ() << ", isPoton: " << isPhoton << std::endl;
#endif

		try
		{
		    pandora::ClusterFitResult clusterFitResult;
		    pandora::ClusterFitHelper::FitFullCluster(pCluster, clusterFitResult);
		    const pandora::CartesianVector& cluDirection = clusterFitResult.GetDirection();
		    const pandora::CartesianVector& cluIntercept = clusterFitResult.GetIntercept();
		    //std::cout << "  *** direction_f: " << cluDirection.GetX() << ", " << cluDirection.GetY() << ", " << cluDirection.GetZ() << std::endl;
		    //std::cout << "  *** intercept_f: " << cluIntercept.GetX() << ", " << cluIntercept.GetY() << ", " << cluIntercept.GetZ() << std::endl;
			
			pCluster->SetAxis(cluDirection);
			pCluster->SetIntercept(cluIntercept);
		}
		catch(pandora::StatusCodeException &)
		{
			//std::cout << "Fit failed, cluster: " << pCluster << ", E: " << pCluster->GetHadronicEnergy() << std::endl;
			//continue;
		}

		try
		{
		    pandora::ClusterFitResult clusterFitResult;
			pandora::ClusterFitHelper::FitStart(pCluster, 3, clusterFitResult);
		    const pandora::CartesianVector& startingPoint = clusterFitResult.GetIntercept();

		    //std::cout << "  *** startingPoint: " << startingPoint.GetX() << ", " << startingPoint.GetY() << ", " << startingPoint.GetZ() << std::endl;

			pCluster->SetStartingPoint(startingPoint);
		}
		catch(pandora::StatusCodeException &)
		{
			//std::cout << "Fit failed, cluster: " << pCluster << ", E: " << pCluster->GetHadronicEnergy() << std::endl;
			//continue;
		}

		try
		{
		    pandora::ClusterFitResult clusterFitResult;
			pandora::ClusterFitHelper::FitEnd(pCluster, 3, clusterFitResult);
		    const pandora::CartesianVector& endpoint = clusterFitResult.GetIntercept();

		    //std::cout << "  *** endpoint: " << endpoint.GetX() << ", " << endpoint.GetY() << ", " << endpoint.GetZ() << std::endl;

			pCluster->SetEndpoint(endpoint);
		}
		catch(pandora::StatusCodeException &)
		{
			//std::cout << "Fit failed, cluster: " << pCluster << ", E: " << pCluster->GetHadronicEnergy() << std::endl;
		}

		m_clustersToMerge.push_back(pCluster);
	}

	std::sort(m_clustersToMerge.begin(), m_clustersToMerge.end(), pandora_monitoring::PandoraMonitoring::SortClustersByHadronicEnergy);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<pandora::CartesianVector> m_clusterCentroids;

	for(auto clu : m_clustersToMerge)
	{
		auto& centroid = clu->GetCentroid();
		m_clusterCentroids.push_back(centroid);
	}

	CaloHitRangeSearchHelper::FillMatixByPoints(m_clusterCentroids, m_clusterCentroidsMatrix);

	std::cout << " === m_clusterCentroids size: " << m_clusterCentroids.size() << std::endl;
	
	//MergeFragmentsByMC(m_clustersToMerge);

#if 1
	for(auto cluster : m_clustersToMerge)
	{
		if(cluster->HasMotherAtSearch()) continue;

		auto arborCluster = ArborContentApi::Modifiable(dynamic_cast<const arbor_content::ArborCluster*>(cluster));
		std::vector<ArborCluster*> properClusters;
		SearchProperClusters(arborCluster, properClusters);

		if(properClusters.size()>0)
		{
			arborCluster->SetRoot();
		}
	}

	// clean clusters
	CleanClusterForMerging(m_clustersToMerge);
#endif
	
    return pandora::STATUS_CODE_SUCCESS;
  }

  void NearbyFragmentsMergingAlgorithm::MergeFragmentsByMC(std::vector<ArborCluster*>& fragmentsToMerge) 
  {
	 std::map<const pandora::MCParticle* const, pandora::ClusterList> mcpClusterListMap;

	 for(int i = 0; i < fragmentsToMerge.size(); ++i)
	 {
	   const pandora::Cluster* const clu = fragmentsToMerge.at(i);
	   const pandora::MCParticle* pClusterMCParticle = nullptr;

       try
       {
       	 pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle( clu );
       }
       catch (pandora::StatusCodeException &)
       {
	       continue;
	   }

	   if(pClusterMCParticle != nullptr && mcpClusterListMap.find( pClusterMCParticle ) == mcpClusterListMap.end())
	   {
	       pandora::ClusterList clusterList;
	       clusterList.push_back( clu );
	       mcpClusterListMap[pClusterMCParticle] = clusterList;
	   }
	   else
	   {
	       mcpClusterListMap[pClusterMCParticle].push_back( clu );
	   }
	 }

	for(auto it = mcpClusterListMap.begin(); it != mcpClusterListMap.end(); ++it)
	{
		// merge charged clusters
		auto mcp = it->first;
		int clusterMCPCharge = pandora::PdgTable::GetParticleCharge(mcp->GetParticleId());
		bool isPhoton = mcp->GetParticleId() == 22; 
		bool notMerge = isPhoton || clusterMCPCharge != 0;

		if(notMerge) 
		{
			//continue;
		}

		auto clusterList = it->second;

		pandora::ClusterVector clusterVector;
		clusterVector.insert(clusterVector.begin(), clusterList.begin(), clusterList.end());
		std::sort(clusterVector.begin(), clusterVector.end(), pandora_monitoring::PandoraMonitoring::SortClustersByHadronicEnergy);

	    if(clusterVector.size()>1) 
		{
			auto firstCluster = clusterVector.at(0);

			for(int i = 1; i < clusterVector.size(); ++i)
			{
		        const pandora::Cluster* cluToMerge = clusterVector.at(i);

		        // GetClustersDistance
		        float closestDistance = 1.e6;

		        try
		        {
		            ClusterHelper::GetClosestDistanceApproach(firstCluster, cluToMerge, closestDistance, false);
		        }
                catch(pandora::StatusCodeException &)
		        {
		            std::cout << "GetClosestDistanceApproach failed" << std::endl;
		        }

				if(closestDistance>40.) continue;

#if 0
				std::cout << " ======= Merge: " << firstCluster << ", E: " << firstCluster->GetHadronicEnergy()
					      << ", " << cluToMerge << ", E: " << cluToMerge->GetHadronicEnergy() 
					      << ", distance: " << closestDistance << ", charge: " << clusterMCPCharge << std::endl;
#endif

			    ArborContentApi::MergeAndDeleteClusters(*this, firstCluster, cluToMerge);
			}
		}
	}
  }

  void NearbyFragmentsMergingAlgorithm::SearchProperClusters(ArborCluster* startingCluster, 
		  std::vector<arbor_content::ArborCluster*>& properClusters)
  {
#if __DEBUG__
	  const pandora::Cluster* const pandoraTrackStartClu = dynamic_cast<const pandora::Cluster* const>(startingCluster);
	  float startCluEnergy = startingCluster->GetHadronicEnergy();

	  auto pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(pandoraTrackStartClu);
	  std::cout << " SearchProperClusters2: cluster: " << startingCluster << ", Ehad: " << startCluEnergy << ", MCP: " << pClusterMCParticle << std::endl;
#endif

	  std::vector<arbor_content::ArborCluster*> nearbyClusters;
	  GetNearbyClusters(startingCluster, m_clustersToMerge, nearbyClusters);

	  // map for sorting all nearby clusters by closest distance
	  std::multimap<float, ArborCluster*> clusterDistanceMap;

	  for(int i = 0; i < nearbyClusters.size(); ++i)
	  {
		  auto nearbyCluster = nearbyClusters.at(i);

		  if(nearbyCluster->HasMotherAtSearch() || nearbyCluster == startingCluster || nearbyCluster->IsRoot())
		  {
			  continue;
		  }

#if 0
		  // angle selection
		  pandora::CartesianVector trackPointAtCalo = pTrack->GetTrackStateAtCalorimeter().GetPosition();
		  pandora::CartesianVector trackMomentumAtCalo = pTrack->GetTrackStateAtCalorimeter().GetMomentum();

		  pandora::CartesianVector trackPointAtCaloClusterDistance = nearbyCluster->GetCentroid() - trackPointAtCalo;

		  float clusterTrackAngle = trackPointAtCaloClusterDistance.GetOpeningAngle(trackMomentumAtCalo);
		  float m_maxClusterTrackAngle = 0.3;
		  if(clusterTrackAngle > m_maxClusterTrackAngle || clusterTrackAngle < 0. || isnan(clusterTrackAngle)) continue;
#endif


		  // GetClustersDistance
		  float closestDistance = 1.e6;

		  try
		  {
			  ClusterHelper::GetClosestDistanceApproach(startingCluster, nearbyCluster, closestDistance, false);
		  }
          catch(pandora::StatusCodeException &)
		  {
			  std::cout << "GetClosestDistanceApproach failed" << std::endl;
		  }

		  if(closestDistance > m_maxClosestFragmentDistance) continue;

#if __DEBUG__
		  std::cout << "nearbyClusters " << i << " : " << nearbyCluster << ", E: " << nearbyCluster->GetHadronicEnergy() << std::endl;
#endif

#if __USEMCP__
		  // help by MC truth
		  try
		  {
			  const pandora::Cluster* const pandoraClu = dynamic_cast<const pandora::Cluster* const>(nearbyCluster);
		      auto pandoraCluMCP = pandora::MCParticleHelper::GetMainMCParticle(pandoraClu);

		      if( pandora::PdgTable::GetParticleCharge(pandoraCluMCP->GetParticleId()) == 0. && 
		          nearbyCluster->GetHadronicEnergy() > 0. )
		      {
		        		continue;
		      }
		  }
		  catch(pandora::StatusCodeException &)
		  {
		  }
#endif

		  //GetClustersDirection
		  auto& startingClusterAxis = startingCluster->GetAxis();

		  //startingCluster->GetAxis();
		  float angle = 1.e6;

		  auto& nearbyClusterCOG = nearbyCluster->GetCentroid();
		  auto& startingClusterCOG = startingCluster->GetCentroid();
		  auto directionOfCentroids = nearbyClusterCOG - startingClusterCOG;

		  if( directionOfCentroids.GetMagnitudeSquared() * startingClusterAxis.GetMagnitudeSquared() > std::numeric_limits<float>::epsilon() )
		  {
			  try
			  {
				  angle = directionOfCentroids.GetOpeningAngle(startingClusterAxis);
			  }
			  catch(pandora::StatusCodeException &)
			  {
				  std::cout << "GetOpeningAngle failed" << std::endl;
			  }
		  }

		  auto& nearbyClusterAxis = nearbyCluster->GetAxis();
		  auto directionsCrossProd = nearbyClusterAxis.GetCrossProduct(startingClusterAxis);
		  float axisDistance = fabs(directionsCrossProd.GetDotProduct(directionOfCentroids)) / directionsCrossProd.GetMagnitude();

		  std::vector<float> clusterParameters;
		  clusterParameters.push_back(closestDistance);
		  clusterParameters.push_back(angle); // axis angle
		  clusterParameters.push_back(axisDistance);

		  std::vector<float> parameterPowers;
		  parameterPowers.push_back(5.);
		  parameterPowers.push_back(3.);
		  parameterPowers.push_back(1.);

		  ClustersOrderParameter orderParameter(clusterParameters, parameterPowers);
		  nearbyCluster->SetOrderParameterWithMother(startingCluster, orderParameter);

		  clusterDistanceMap.insert( std::pair<float, ArborCluster*>(closestDistance, nearbyCluster) );
      }
		  
	  for(auto it = clusterDistanceMap.begin(); it != clusterDistanceMap.end(); ++it)
	  {
		  auto nearbyCluster = it->second;

		  properClusters.push_back(nearbyCluster);
		  nearbyCluster->SetHasMotherAtSearch();
	  }

	  startingCluster->SetClustersToMerge(properClusters);
  }

  void NearbyFragmentsMergingAlgorithm::SearchProperClusters(const pandora::Track* pTrack, ArborCluster* startingCluster, 
		  std::vector<arbor_content::ArborCluster*>& properClusters)
  {
	  
#if __DEBUG__
	  const pandora::Cluster* const pandoraTrackStartClu = dynamic_cast<const pandora::Cluster* const>(startingCluster);
	  float startCluEnergy = startingCluster->GetHadronicEnergy();

	  auto pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(pandoraTrackStartClu);
	  std::cout << " SearchProperClusters for charged cluster: " << startingCluster << ", Ehad: " << startCluEnergy << ", MCP: " << pClusterMCParticle << std::endl;
#endif

	  std::vector<arbor_content::ArborCluster*> nearbyClusters;
	  GetNearbyClusters(startingCluster, m_clustersToMerge, nearbyClusters);

	  // map for sorting all nearby clusters by closest distance
	  std::multimap<float, ArborCluster*> clusterDistanceMap;

	  for(int i = 0; i < nearbyClusters.size(); ++i)
	  {
		  auto nearbyCluster = nearbyClusters.at(i);

		  if(nearbyCluster->HasMotherAtSearch() || nearbyCluster == startingCluster || nearbyCluster->IsRoot() || nearbyCluster->IsPhoton()) 
		  {
			  continue;
		  }

#if 0
		  // angle selection
		  pandora::CartesianVector trackPointAtCalo = pTrack->GetTrackStateAtCalorimeter().GetPosition();
		  pandora::CartesianVector trackMomentumAtCalo = pTrack->GetTrackStateAtCalorimeter().GetMomentum();

		  pandora::CartesianVector trackPointAtCaloClusterDistance = nearbyCluster->GetCentroid() - trackPointAtCalo;

		  float clusterTrackAngle = trackPointAtCaloClusterDistance.GetOpeningAngle(trackMomentumAtCalo);
		  float m_maxClusterTrackAngle = 0.3;
		  if(clusterTrackAngle > m_maxClusterTrackAngle || clusterTrackAngle < 0. || isnan(clusterTrackAngle)) continue;
#endif

#if __DEBUG__
		  std::cout << "nearbyClusters " << i << " : " << nearbyCluster << ", E: " << nearbyCluster->GetHadronicEnergy() << std::endl;
#endif


		  // GetClustersDistance
		  float closestDistance = 1.e6;

		  try
		  {
			  ClusterHelper::GetClosestDistanceApproach(startingCluster, nearbyCluster, closestDistance, false);
		  }
          catch(pandora::StatusCodeException &)
		  {
			  std::cout << "GetClosestDistanceApproach failed" << std::endl;
		  }

		  // FIXME
		  float emEnergyInECAL = ClusterHelper::GetElectromagneticEnergyInECAL(nearbyCluster);
		  float emEnergyRatio  = emEnergyInECAL / nearbyCluster->GetElectromagneticEnergy();

		  float m_maxClosestDistance = 1.e6;
		  float meanDensity = 1.;

		  if(emEnergyRatio > 0.6) 
		  {
		      // mainly in ECAL 
		      m_maxClosestDistance = 100.;

			  ClusterHelper::GetMeanDensity(nearbyCluster, meanDensity);

			  // seems a hadronic fragment
			  if(meanDensity<0.3) m_maxClosestDistance = 200;
		  }
		  else
		  {
		      m_maxClosestDistance = 500.;
		  }

		  if(closestDistance > m_maxClosestDistance) 
		  {
#if __DEBUG__
			  std::cout << "emEnergyRatio: " << emEnergyRatio << ", m_maxClosestDistance: " << m_maxClosestDistance 
				  << ", meanDensity: " << meanDensity << ", closestDistance: " << closestDistance << std::endl;
#endif

			  continue;
		  }
		
#if __USEMCP__
		  // help by MC truth
		  try
		  {
			  const pandora::Cluster* const pandoraClu = dynamic_cast<const pandora::Cluster* const>(nearbyCluster);
		      auto pandoraCluMCP = pandora::MCParticleHelper::GetMainMCParticle(pandoraClu);

		      if( pandora::PdgTable::GetParticleCharge(pandoraCluMCP->GetParticleId()) == 0. && 
		          nearbyCluster->GetHadronicEnergy() > 0. )
		      {
		        		continue;
		      }
		  }
		  catch(pandora::StatusCodeException &)
		  {
		  }
#endif

		  //GetClustersDirection
		  auto& startingClusterAxis = startingCluster->GetAxis();

		  //startingCluster->GetAxis();
		  float angle = 1.e6;

		  auto& nearbyClusterCOG = nearbyCluster->GetCentroid();
		  auto& startingClusterCOG = startingCluster->GetCentroid();
		  auto directionOfCentroids = nearbyClusterCOG - startingClusterCOG;

		  if( directionOfCentroids.GetMagnitudeSquared() * startingClusterAxis.GetMagnitudeSquared() > std::numeric_limits<float>::epsilon() )
		  {
			  try
			  {
				  angle = directionOfCentroids.GetOpeningAngle(startingClusterAxis);
			  }
			  catch(pandora::StatusCodeException &)
			  {
				  std::cout << "GetOpeningAngle failed" << std::endl;
			  }
		  }

		  const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField( pandora::CartesianVector(0.f, 0.f, 0.f)));

		  const pandora::Helix helix(pTrack->GetTrackStateAtCalorimeter().GetPosition(),
		    	  pTrack->GetTrackStateAtCalorimeter().GetMomentum(), pTrack->GetCharge(), bField);
		
		  pandora::CartesianVector trackCluCentroidDistanceVec(0., 0., 0.);
		  float genericTime = 0.;

		  if(pandora::STATUS_CODE_SUCCESS != helix.GetDistanceToPoint(nearbyClusterCOG, trackCluCentroidDistanceVec, genericTime))
		  {
			std::cout << "helix.GetDistanceToPoint failed" << std::endl;
		  	continue;
		  }


#if __DEBUG__
		  float trackCluCentroidDistance = trackCluCentroidDistanceVec.GetMagnitude();
	      //const pandora::Cluster* const pandoraNearbyClu = dynamic_cast<const pandora::Cluster* const>(nearbyCluster);
	      //auto nearbyClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(pandoraNearbyClu);
		  float nearbyCluEnergy = nearbyCluster->GetHadronicEnergy();

		  std::cout << " --- clu: " << nearbyCluster << ", E: " << nearbyCluEnergy
		   			<< ", trackCluCentroidDistance: " << trackCluCentroidDistance << ", angle: " << angle << std::endl;
#endif

#if 0
		  bool isGoodAngle = (angle < 0.3) ;
		  auto pseudoLayerPlugin = PandoraContentApi::GetPlugins(*this)->GetPseudoLayerPlugin();
		  const unsigned int startingLayer1(pseudoLayerPlugin->GetPseudoLayer(startingCluster->GetStartingPoint()));
		  const unsigned int startingLayer2(pseudoLayerPlugin->GetPseudoLayer(nearbyCluster->GetStartingPoint()));

		  if(isGoodAngle) 
		  if( (startingLayer1 < startingLayer2) && (trackCluCentroidDistance < 10. || isGoodAngle) ) 
#endif

		  auto& nearbyClusterAxis = nearbyCluster->GetAxis();
		  auto directionsCrossProd = nearbyClusterAxis.GetCrossProduct(startingClusterAxis);
		  float axisDistance = fabs(directionsCrossProd.GetDotProduct(directionOfCentroids)) / directionsCrossProd.GetMagnitude();

		  std::vector<float> clusterParameters;

		  clusterParameters.push_back(closestDistance);
		  clusterParameters.push_back(angle); // axis angle
		  clusterParameters.push_back(axisDistance);

		  std::vector<float> parameterPowers;
		  parameterPowers.push_back(5.);
		  parameterPowers.push_back(3.);
		  parameterPowers.push_back(1.);

		  ClustersOrderParameter orderParameter(clusterParameters, parameterPowers);
		  nearbyCluster->SetOrderParameterWithMother(startingCluster, orderParameter);

		  clusterDistanceMap.insert( std::pair<float, ArborCluster*>(closestDistance, nearbyCluster) );
      }
		  
	  for(auto it = clusterDistanceMap.begin(); it != clusterDistanceMap.end(); ++it)
	  {
		  auto nearbyCluster = it->second;

		  properClusters.push_back(nearbyCluster);
		  nearbyCluster->SetHasMotherAtSearch();
	  }

	  startingCluster->SetClustersToMerge(properClusters);
		
#if 0
	  // search proper cluster's proper cluster
	  for(int iClu = 0; iClu < properClusters.size(); ++iClu)
	  {
		  auto clu = properClusters.at(iClu);
		  
		  std::vector<ArborCluster*> clusters;
		  SearchProperClusters(pTrack, clu, clusters);
	  }
#endif
		  
#if __DEBUG__
	  std::cout << "-----------------------------------------------------------------------------------------------------------" << std::endl;
#endif
  }

  void NearbyFragmentsMergingAlgorithm::GetNearbyClusters(pandora::Cluster* cluster, 
		  const std::vector<arbor_content::ArborCluster*>& clusterVector, std::vector<arbor_content::ArborCluster*>& clustersInRange)
  {
      pandora::CartesianVector centroid(0., 0., 0.);
	  ClusterHelper::GetCentroid(cluster, centroid);

	  const float distance = m_maxStartingClusterDistance;
	  const mlpack::math::Range range(0., distance);

      arma::mat testPoint(3, 1);
	  testPoint.col(0)[0] = centroid.GetX();
	  testPoint.col(0)[1] = centroid.GetY();
	  testPoint.col(0)[2] = centroid.GetZ();
	  
	  mlpack::range::RangeSearch<> rangeSearch(m_clusterCentroidsMatrix);
      std::vector<std::vector<size_t> > resultingNeighbors;
      std::vector<std::vector<double> > resultingDistances;
      rangeSearch.Search(testPoint, range, resultingNeighbors, resultingDistances);

      std::vector<size_t>& neighbors = resultingNeighbors.at(0);
      std::vector<double>& distances = resultingDistances.at(0);
	  
      for(size_t j=0; j < neighbors.size(); ++j)
      {
      	size_t neighbor = neighbors.at(j);
      	//double hitsDist = distances.at(j);

		clustersInRange.push_back( clusterVector.at(neighbor) );
	  }

	  // sort cluster by distance
	  std::multimap<float, ArborCluster*> clusterDistanceMap;

	  for(int i = 0; i < clustersInRange.size(); ++i)
	  {
		  auto clusterInRange = clustersInRange.at(i);

		  clusterDistanceMap.insert( std::pair<float, ArborCluster*>(distances.at(i), clusterInRange) );
	  }

	  clustersInRange.clear();

	  for(auto& mapIter : clusterDistanceMap)
	  {
		  auto clu = mapIter.second;
		  clustersInRange.push_back(clu);
	  }

	  ///////////////////////////////////////////////////////////////////////////////////////////////
		
#if 0
	  auto pClusterMCP = pandora::MCParticleHelper::GetMainMCParticle(cluster);

	  std::cout << "------------ cluster: " << cluster << ", energy: " << cluster->GetHadronicEnergy() 
		  << ", MCP: " << pClusterMCP << ", nearby clusters: " << clustersInRange.size() << std::endl;

	  for(auto it = clusterDistanceMap.begin(); it != clusterDistanceMap.end(); ++it)
	  {
		  //auto pCluster = clustersInRange.at(i);
		  auto distance = it->first;
		  auto pCluster = it->second;


		  const pandora::Cluster* const clu = dynamic_cast<const pandora::Cluster* const>(pCluster);
		  bool isPhoton = PandoraContentApi::GetPlugins(*this)->GetParticleId()->IsPhoton(clu);
		  auto pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(clu);

		  std::cout << " *clu: " << clu << ", dist: " << distance << ", nhits: " << 
			 pCluster->GetNCaloHits() << ", Ehad: " << pCluster->GetHadronicEnergy() 
			 << ", iL: " << pCluster->GetInnerPseudoLayer() << ": isPhoton: " << pCluster->IsPhoton() 
			 << ": MCP: " << pClusterMCParticle << std::endl;
	  }
#endif
  }

  pandora::StatusCode NearbyFragmentsMergingAlgorithm::CleanClusterForMerging(std::vector<ArborCluster*>& clusterVector)
  {
	for(int i = 0; i < clusterVector.size(); ++i)
	{
		auto& cluster = clusterVector.at(i);

		auto& mothers = cluster->GetMotherCluster();

#if __DEBUG1__
		std::cout << " --- cluster " << cluster << " mothers: " << mothers.size() << ", root?: " << cluster->IsRoot() << std::endl;
#endif

		// find the best one
		ClustersOrderParameter bestOrderParameter;
		ArborCluster* bestCluster;

		for(int iMother = 0; iMother < mothers.size(); ++iMother)
		{
			auto mother = mothers.at(iMother);
			ClustersOrderParameter orderParameter = cluster->GetOrderParameterWithMother(mother);

			if(orderParameter < bestOrderParameter)
			{
				bestOrderParameter = orderParameter;
				bestCluster = mother;
			}
		}

		// take the best one
		if(mothers.size() > 1)
		{
			for(int iMother = 0; iMother < mothers.size(); ++iMother)
			{
				auto mother = mothers.at(iMother);

				if(mother != bestCluster)
				{
#if __DEBUG__
				    std::cout << " !!! cluster: " << mother << " remove cluster to merge: " << cluster << std::endl;
#endif
					mother->RemoveFromClustersToMerge(cluster);
				}
			}

			mothers.clear();
			mothers.push_back(bestCluster);
		}
	}

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode NearbyFragmentsMergingAlgorithm::GetEligibleClusters(std::vector<ArborCluster*>& clusterVector) const
  {
	clusterVector.clear();

    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	//std::cout << ">>>cluster number: " << pClusterList->size() << std::endl;

    if(pClusterList->empty())
      return pandora::STATUS_CODE_SUCCESS;

    for(pandora::ClusterList::const_iterator clusterIter = pClusterList->begin(), clusterEndIter = pClusterList->end() ;
        clusterEndIter != clusterIter ; ++clusterIter)
    {
      const pandora::Cluster *const pCluster = *clusterIter;

      //if(!this->CanMergeCluster(pCluster))
      //  continue;

	  auto arborCluster = ArborContentApi::Modifiable(dynamic_cast<const arbor_content::ArborCluster*>(pCluster));
	  arborCluster->Reset();
      clusterVector.push_back(arborCluster);
    }

    // sort them by inner layer
    std::sort(clusterVector.begin(), clusterVector.end(), SortingHelper::SortClustersByInnerLayer);

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode NearbyFragmentsMergingAlgorithm::FindBestParentCluster(const pandora::Cluster *const pDaughterCluster, const pandora::ClusterVector &clusterVector,
      const pandora::Cluster *&pBestParentCluster) const
  {
    pBestParentCluster = NULL;

    if(NULL == pDaughterCluster)
      return pandora::STATUS_CODE_INVALID_PARAMETER;

    pandora::CartesianVector innerPosition(0.f, 0.f, 0.f), backwardDirection(0.f, 0.f, 0.f);
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetClusterBackwardDirection(pDaughterCluster, backwardDirection, innerPosition));
    const unsigned int innerPseudoLayer(PandoraContentApi::GetPlugins(*this)->GetPseudoLayerPlugin()->GetPseudoLayer(innerPosition));

    const pandora::Granularity granularity(this->GetPandora().GetGeometry()->GetHitTypeGranularity(pDaughterCluster->GetInnerLayerHitType()));
    const float maxClusterDistance(granularity <= pandora::FINE ? m_maxBackwardDistanceFine : m_maxBackwardDistanceCoarse);
    const unsigned int maxPseudoLayerDifference(m_maxBackwardPseudoLayer);
    float bestClusterCosineAngle(std::numeric_limits<float>::min());

    for(pandora::ClusterVector::const_reverse_iterator jIter = clusterVector.rbegin(), jEndIter = clusterVector.rend() ;
        jEndIter != jIter ; ++jIter)
    {
      const pandora::Cluster *const pCluster(*jIter);

      if(NULL == pCluster)
        continue;

      if(pCluster == pDaughterCluster)
        continue;

      if(pCluster->GetAssociatedTrackList().empty())
        continue;
	  else
		  std::cout << "---cluster tracks: " << pCluster->GetAssociatedTrackList().size() << std::endl;

      const pandora::Track *pTrack((*pCluster->GetAssociatedTrackList().begin()));
      const pandora::CartesianVector trackMomentum(pTrack->GetTrackStateAtCalorimeter().GetMomentum());
      const pandora::CartesianVector trackProjection(pTrack->GetTrackStateAtCalorimeter().GetPosition());
      const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));
      const pandora::Helix helix(trackProjection, trackMomentum, pTrack->GetCharge(), bField);

      // compute parent cluster end point using cluster fit
      pandora::CartesianVector centroid(0.f, 0.f, 0.f);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetCentroid(pCluster, centroid));

      const pandora::CartesianVector outerCentroid(pCluster->GetCentroid(pCluster->GetOuterPseudoLayer()));
      const unsigned int outerParentPseudoLayer(pCluster->GetOuterPseudoLayer());
      const unsigned int innerParentPseudoLayer(pCluster->GetInnerPseudoLayer());
      const unsigned int clusterPseudoLayerExtension(outerParentPseudoLayer-innerParentPseudoLayer);
      const unsigned int nFitPseudoLayers(clusterPseudoLayerExtension >= m_nBackwardLayersFit ? m_nBackwardLayersFit : clusterPseudoLayerExtension);

      pandora::ClusterFitResult clusterFitResult;
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::ClusterFitHelper::FitEnd(pCluster, nFitPseudoLayers, clusterFitResult));
      const pandora::CartesianVector clusterDirection(clusterFitResult.GetDirection());

      pandora::CartesianVector clusterEndPoint(0.f, 0.f, 0.f);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, GeometryHelper::GetProjectionOnLine(centroid, clusterDirection, outerCentroid, clusterEndPoint));

      pandora::CartesianVector trackEndPointPosition(0.f, 0.f, 0.f), trackEndPointMomentum(0.f, 0.f, 0.f);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, GeometryHelper::GetProjectionOnHelix(helix, clusterEndPoint, trackEndPointPosition));
      trackEndPointMomentum = helix.GetExtrapolatedMomentum(trackEndPointPosition);

      // possible parent if :
      // 1) cluster distance if OK AND angle between fits is ok (cut on cosine)
      // 2) cluster distance if OK AND angle between fits is compatible (cut relaxed) AND dca between fits is OK

      // check pseudo layer cluster separation
      const unsigned int endPseudoLayer(PandoraContentApi::GetPlugins(*this)->GetPseudoLayerPlugin()->GetPseudoLayer(clusterEndPoint));
      const unsigned int pseudoLayerDifference(std::max(innerPseudoLayer, endPseudoLayer) - std::min(innerPseudoLayer, endPseudoLayer));

      if(endPseudoLayer >= innerPseudoLayer || pseudoLayerDifference > maxPseudoLayerDifference)
        continue;

      // distance between clusters
      const pandora::CartesianVector clusterDifferenceDirection(innerPosition-clusterEndPoint);
      const float clusterDistance(clusterDifferenceDirection.GetMagnitude());

      if(clusterDistance > maxClusterDistance)
        continue;

      // compute cluster-to-cluster properties
      const float clusterFitCosineAngle(clusterDirection.GetCosOpeningAngle(backwardDirection * -1.));
      const float clusterCosineAngle(trackEndPointMomentum.GetCosOpeningAngle(clusterDifferenceDirection));

      float clusterFitDca(0.f);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, GeometryHelper::GetClosestDistanceBetweenLines(clusterEndPoint, clusterDirection, innerPosition, backwardDirection, clusterFitDca));

      bool possibleAssociation(false);

      // contained in small angle
      if(clusterFitCosineAngle > m_minClusterFitCosOpeningAngle2 && clusterCosineAngle > m_minClusterCosOpeningAngle)
      {
        possibleAssociation = true;
      }
      // contained in larger angle but short dca
      else if(clusterFitDca < m_maxClusterFitDca && clusterFitCosineAngle > m_minClusterFitCosOpeningAngle && clusterCosineAngle > m_minClusterCosOpeningAngle)
      {
        possibleAssociation = true;
      }

      if(possibleAssociation)
      {
        float oldChi(0.f), newChi(0.f);
		float trackEnergy(0.f);

        if(pandora::STATUS_CODE_SUCCESS != ClusterHelper::GetChiClusterMerging(this->GetPandora(), pCluster, pDaughterCluster, trackEnergy, oldChi, newChi))
          continue;

        const bool angleTightened(clusterCosineAngle > bestClusterCosineAngle);
        const bool chi2Compatible(newChi*newChi < oldChi*oldChi || newChi*newChi < m_chi2AssociationCut);

        if(angleTightened && chi2Compatible)
        {
          bestClusterCosineAngle = clusterCosineAngle;
          pBestParentCluster = pCluster;
        }
      }
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode NearbyFragmentsMergingAlgorithm::GetClusterBackwardDirection(const pandora::Cluster *const pCluster, pandora::CartesianVector &backwardDirection, pandora::CartesianVector &innerPosition) const
  {
    pandora::CartesianVector centroid(0.f, 0.f, 0.f);
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetCentroid(pCluster, centroid));

    const pandora::CartesianVector innerCentroid(pCluster->GetCentroid(pCluster->GetInnerPseudoLayer()));

    pandora::ClusterFitResult clusterFitResult;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::ClusterFitHelper::FitStart(pCluster, m_nBackwardLayersFit, clusterFitResult));
    const pandora::CartesianVector clusterDirection(clusterFitResult.GetDirection());

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, GeometryHelper::GetProjectionOnLine(centroid, clusterDirection, innerCentroid, innerPosition));
    backwardDirection = clusterDirection * -1.f;

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode NearbyFragmentsMergingAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {

    m_discriminatePhotonPid = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DiscriminatePhotonPid", m_discriminatePhotonPid));

    m_allowNeutralParentMerging = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "AllowNeutralParentMerging", m_allowNeutralParentMerging));

    m_minNCaloHits = 0;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinNCaloHits", m_minNCaloHits));

    m_maxNCaloHits = std::numeric_limits<unsigned int>::max();
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNCaloHits", m_maxNCaloHits));

    m_minNPseudoLayers = 2;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinNPseudoLayers", m_minNPseudoLayers));

    m_maxNPseudoLayers = std::numeric_limits<unsigned int>::max();
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNPseudoLayers", m_maxNPseudoLayers));

    m_chi2AssociationCut = 1.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "Chi2AssociationCut", m_chi2AssociationCut));

    m_nBackwardLayersFit = 6;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "NBackwardLayersFit", m_nBackwardLayersFit));

    m_maxBackwardAngle = 0.4;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxBackwardAngle", m_maxBackwardAngle));

    m_maxBackwardDistanceFine = 300.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxBackwardDistanceFine", m_maxBackwardDistanceFine));

    m_maxBackwardDistanceCoarse = 1000.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxBackwardDistanceCoarse", m_maxBackwardDistanceCoarse));

    m_maxBackwardPseudoLayer = 20;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxBackwardPseudoLayer", m_maxBackwardPseudoLayer));

    m_minParentClusterBackwardNHits = 5;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinParentClusterBackwardNHits", m_minParentClusterBackwardNHits));

    m_maxClusterFitDca = 25.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxClusterFitDca", m_maxClusterFitDca));

    m_minClusterCosOpeningAngle = 0.7;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterCosOpeningAngle", m_minClusterCosOpeningAngle));

    m_minClusterFitCosOpeningAngle = 0.7;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterFitCosOpeningAngle", m_minClusterFitCosOpeningAngle));

    m_minClusterFitCosOpeningAngle2 = 0.8;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterFitCosOpeningAngle2", m_minClusterFitCosOpeningAngle2));

    m_maxStartingClusterDistance = 2000.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxStartingClusterDistance", m_maxStartingClusterDistance));

	m_maxClusterDistanceToMerge = 15.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterDistanceToMerge", m_maxClusterDistanceToMerge));
		  
	m_maxClosestFragmentDistance = 50.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxClosestFragmentDistance", m_maxClosestFragmentDistance));

	m_mergePhotonClusters = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MergePhotonClusters", m_mergePhotonClusters));

	m_mergeChargedClusters = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MergeChargedClusters", m_mergeChargedClusters));

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

