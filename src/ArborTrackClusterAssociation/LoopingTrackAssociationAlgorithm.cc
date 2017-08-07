/// \file LoopingTrackAssociationAlgorithm.cc
/*
 *
 * LoopingTrackAssociationAlgorithm.cc source template automatically generated by a class generator
 * Creation date : mer. oct. 5 2016
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

#include "ArborTrackClusterAssociation/LoopingTrackAssociationAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"
#include "ArborHelpers/ReclusterHelper.h"
#include "ArborHelpers/ClusterHelper.h"

namespace arbor_content
{

  pandora::StatusCode LoopingTrackAssociationAlgorithm::Run()
  {
    const pandora::TrackList *pTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pTrackList));

    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    const float ecalEndcapInnerZCoordinate(PandoraContentApi::GetGeometry(*this)->GetSubDetector(pandora::ECAL_ENDCAP).GetInnerZCoordinate());
    const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));

	int nLoopingTrack(0);

    for(pandora::TrackList::const_iterator iter = pTrackList->begin(), endIter = pTrackList->end() ;
        endIter != iter ; ++iter)
    {
      const pandora::Track *const pTrack(*iter);

      if(!pTrack->CanFormPfo() || !pTrack->ReachesCalorimeter())
        continue;

      if(!pTrack->GetDaughterList().empty() || pTrack->HasAssociatedCluster())
        continue;

      const pandora::Helix helix(pTrack->GetTrackStateAtCalorimeter().GetPosition(),
          pTrack->GetTrackStateAtCalorimeter().GetMomentum(), pTrack->GetCharge(), bField);

      if(fabs(helix.GetOmega()) < m_maxOmegaTrack)
        continue;

      if(!pTrack->IsProjectedToEndCap())
        continue;

      const float trackProjectionZ(pTrack->GetTrackStateAtCalorimeter().GetPosition().GetZ());

      // search for a cluster to associate
      const pandora::Cluster *pBestCluster = NULL;
      float bestXYDistance(std::numeric_limits<float>::max());
      float bestChi2(std::numeric_limits<float>::max());

      for(pandora::ClusterList::const_iterator clusterIter = pClusterList->begin(), clusterEndIter = pClusterList->end() ;
          clusterEndIter != clusterIter ; ++clusterIter)
      {
        const pandora::Cluster *const pCluster(*clusterIter);

        if(!pCluster->GetAssociatedTrackList().empty())
          continue;

        if(pCluster->PassPhotonId(this->GetPandora()))
          continue;

        const pandora::CartesianVector clusterInnerPosition(pCluster->GetCentroid(pCluster->GetInnerPseudoLayer()));
        const pandora::Granularity granularity(PandoraContentApi::GetGeometry(*this)->GetHitTypeGranularity(pCluster->GetInnerLayerHitType()));

        // check region compatibility
        if(trackProjectionZ * clusterInnerPosition.GetZ() < 0)
          continue;

        // must be in endcap
        if(fabs(clusterInnerPosition.GetZ()) < fabs(ecalEndcapInnerZCoordinate))
          continue;

        const float x(clusterInnerPosition.GetX()), y(clusterInnerPosition.GetY());

        // calculate track position at cluster starting point
        pandora::CartesianVector trackCrossingPoint(0.f, 0.f, 0.f);
        if (pandora::STATUS_CODE_SUCCESS != helix.GetPointInZ(clusterInnerPosition.GetZ(), helix.GetReferencePoint(), trackCrossingPoint))
          continue;

        const float trackX(trackCrossingPoint.GetX()), trackY(trackCrossingPoint.GetY());
        const float xyDistance(std::sqrt((x-trackX)*(x-trackX) + (y-trackY)*(y-trackY)));
        const float maxXYDistance(granularity <= pandora::FINE ? m_maxXYDistanceFine : m_maxXYDistanceCoarse);

        if(xyDistance > maxXYDistance)
          continue;

        // Calculate direction of first n layers of cluster
        pandora::ClusterFitResult clusterFitResult;
        if (pandora::STATUS_CODE_SUCCESS != pandora::ClusterFitHelper::FitStart(pCluster, m_nClusterFitLayers, clusterFitResult))
            continue;

        if(!clusterFitResult.IsFitSuccessful())
          continue;

        // calculate track direction
        const pandora::CartesianVector &clusterDirection(clusterFitResult.GetDirection());
        const pandora::CartesianVector trackDirection(helix.GetExtrapolatedMomentum(trackCrossingPoint));
        const float cosDirection(trackDirection.GetCosOpeningAngle(clusterDirection));

        if(cosDirection < m_minCosDirection)
          continue;

        pandora::TrackList trackList;
        trackList.push_back(pTrack);

        // calculate track-cluster compatibility
        const float chi(ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), pCluster, trackList));

        if( (xyDistance < bestXYDistance) || (xyDistance == bestXYDistance && chi*chi < bestChi2) )
        {
          bestXYDistance = xyDistance;
          bestChi2 = chi*chi;
          pBestCluster = pCluster;
        }
      }

      if(NULL != pBestCluster)
      {
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddTrackClusterAssociation(*this, pTrack, pBestCluster));
		++nLoopingTrack;

		//std::cout << "Looping track " << nLoopingTrack << ", energy: " << pTrack->GetEnergyAtDca() << std::endl;
      }
    }
    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode LoopingTrackAssociationAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_maxOmegaTrack = 0.002;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxOmegaTrack", m_maxOmegaTrack));

    m_nClusterFitLayers = 5;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "NClusterFitLayers", m_nClusterFitLayers));

    m_maxInnerClusterPseudoLayer = 5;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxInnerClusterPseudoLayer", m_maxInnerClusterPseudoLayer));

    m_maxInnerClusterPseudoLayerInnerRegion = 15;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxInnerClusterPseudoLayerInnerRegion", m_maxInnerClusterPseudoLayerInnerRegion));

    m_maxXYDistanceFine = 15.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxXYDistanceFine", m_maxXYDistanceFine));

    m_maxXYDistanceCoarse = 30.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxXYDistanceCoarse", m_maxXYDistanceCoarse));

    m_minCosDirection = 0.8;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinCosDirection", m_minCosDirection));

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

