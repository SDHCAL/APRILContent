/// \file NearbyTrackPhotonRemovalAlg.cc
/*
 *
 * NearbyTrackPhotonRemovalAlg.cc source template automatically generated by a class generator
 * Creation date : lun. juin 27 2016
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


#include "APRILTopologicalAssociation/NearbyTrackPhotonRemovalAlg.h"

#include "Pandora/AlgorithmHeaders.h"

#include "APRILApi/APRILContentApi.h"
#include "APRILObjects/CaloHit.h"
#include "APRILHelpers/ClusterHelper.h"
#include "APRILHelpers/GeometryHelper.h"

namespace april_content
{

  pandora::StatusCode NearbyTrackPhotonRemovalAlg::Run()
  {
    // get algorithm contents
    pandora::ClusterVector clusterVector; pandora::TrackVector trackVector;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetContents(clusterVector, trackVector));

    if(clusterVector.empty() || trackVector.empty())
      return pandora::STATUS_CODE_SUCCESS;

    pandora::ClusterVector photonFragmentVector;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->RemoveNearbyPhotonHits(clusterVector, trackVector, photonFragmentVector));
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->RemovePhotonFragments(photonFragmentVector));

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode NearbyTrackPhotonRemovalAlg::GetContents(pandora::ClusterVector &clusterVector, pandora::TrackVector &trackVector) const
  {
    // Get photon cluster list
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_photonClusterListName, pClusterList));

    clusterVector.insert(clusterVector.end(), pClusterList->begin(), pClusterList->end());

	std::cout << "clusterVector size: " << clusterVector.size() << std::endl;

	if(clusterVector.empty()) 
	{
		return pandora::STATUS_CODE_SUCCESS;
	}
    // Get the track list to be used as veto
    const pandora::TrackList *pTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pTrackList));

    const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));

    for(pandora::TrackList::const_iterator trackIter = pTrackList->begin(), trackEndIter = pTrackList->end() ;
        trackEndIter != trackIter ; ++trackIter)
    {
      const pandora::Track *const pTrack(*trackIter);

      if(!pTrack->CanFormPfo())
        continue;

      if(!pTrack->ReachesCalorimeter())
        continue;

      if(!pTrack->GetDaughterList().empty())
        continue;

      const pandora::Helix helix(pTrack->GetTrackStateAtCalorimeter().GetPosition(),
          pTrack->GetTrackStateAtCalorimeter().GetMomentum(), pTrack->GetCharge(), bField);

      if(fabs(helix.GetOmega()) > m_maxOmegaTrack)
        continue;

      trackVector.push_back(pTrack);
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode NearbyTrackPhotonRemovalAlg::RemoveNearbyPhotonHits(const pandora::ClusterVector &clusterVector, const pandora::TrackVector &trackVector,
      pandora::ClusterVector &photonFragmentVector) const
  {
    const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));
    unsigned int totalRemovedNHits(0); // just for logging

    for(pandora::ClusterVector::const_iterator clusterIter = clusterVector.begin(), clusterEndIter = clusterVector.end() ; clusterEndIter != clusterIter ; ++clusterIter)
    {
      const pandora::Cluster *const pCluster(*clusterIter);

      pandora::CaloHitList clusterCaloHits;
      pCluster->GetOrderedCaloHitList().FillCaloHitList(clusterCaloHits);

      pandora::CaloHitList caloHitListRemoval;

      for(pandora::TrackVector::const_iterator trackIter = trackVector.begin(), trackEndIter = trackVector.end() ;
          trackEndIter != trackIter ; ++trackIter)
      {
        const pandora::Track *const pTrack(*trackIter);
        const pandora::CartesianVector &trackProjection(pTrack->GetTrackStateAtCalorimeter().GetPosition());

        const pandora::Helix helix(pTrack->GetTrackStateAtCalorimeter().GetPosition(),
            pTrack->GetTrackStateAtCalorimeter().GetMomentum(), pTrack->GetCharge(), bField);

        for(pandora::CaloHitList::const_iterator iter = clusterCaloHits.begin(), endIter = clusterCaloHits.end() ;
            endIter != iter ; ++iter)
        {
          const pandora::CaloHit *const pCaloHit(*iter);

          const float trackHitAngle(pCaloHit->GetPositionVector().GetOpeningAngle(trackProjection));

          // cut needed for discriminate track opposite direction
          if(trackHitAngle > M_PI/2.f)
            continue;

          pandora::CartesianVector projectionOnHelix(0.f, 0.f, 0.f);

          if(pandora::STATUS_CODE_SUCCESS != GeometryHelper::GetProjectionOnHelix(helix, pCaloHit->GetPositionVector(), projectionOnHelix))
            continue;

          const float distanceToHelix((projectionOnHelix-pCaloHit->GetPositionVector()).GetMagnitude());

          if(distanceToHelix > m_maxTrackHitDistance)
            continue;

          caloHitListRemoval.push_back(pCaloHit);
          ++totalRemovedNHits;
        }
      }

      for(pandora::CaloHitList::const_iterator iter = caloHitListRemoval.begin(), endIter = caloHitListRemoval.end() ;
          endIter != iter ; ++iter)
      {
        const april_content::CaloHit *const pCaloHit(dynamic_cast<const april_content::CaloHit *const>(*iter));

        if(NULL != pCaloHit)
        {
          PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, APRILContentApi::RemoveAndDeleteAllConnections(pCaloHit));
        }

        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RemoveFromCluster(*this, pCluster, *iter));
      }

      const unsigned int nRemainingCaloHits(pCluster->GetNCaloHits());
      const float remainingEnergy(pCluster->GetElectromagneticEnergy());

      if(nRemainingCaloHits < m_minRemainingPhotonNHits || remainingEnergy < m_minRemainingPhotonEnergy)
      {
        photonFragmentVector.push_back(pCluster);
      }
    }

    APRIL_LOG( "NearbyTrackPhotonRemovalAlg: Will remove " << photonFragmentVector.size() << " photons" << std::endl);
    APRIL_LOG( "NearbyTrackPhotonRemovalAlg: Removed " << totalRemovedNHits << " hits from photon clusters" << std::endl);

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode NearbyTrackPhotonRemovalAlg::RemovePhotonFragments(const pandora::ClusterVector &photonFragmentVector) const
  {
    for(pandora::ClusterVector::const_iterator iter = photonFragmentVector.begin(), endIter = photonFragmentVector.end() ; endIter != iter ; ++iter)
    {
      const pandora::Cluster *const pCluster(*iter);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::Delete(*this, pCluster, m_photonClusterListName));
    }
#if 0
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_photonClusterListName, pClusterList));

	std::cout << "the photon cluster number after remove photon fragments: " << pClusterList->size() << std::endl;
#endif

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode NearbyTrackPhotonRemovalAlg::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "PhotonClusterListName", m_photonClusterListName));

    m_maxTrackHitDistance = 8.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTrackHitDistance", m_maxTrackHitDistance));

    m_maxOmegaTrack = 0.002;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxOmegaTrack", m_maxOmegaTrack));

    m_minRemainingPhotonEnergy = 0.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinRemainingPhotonEnergy", m_minRemainingPhotonEnergy));

    m_minRemainingPhotonNHits = 5;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinRemainingPhotonNHits", m_minRemainingPhotonNHits));

    return pandora::STATUS_CODE_SUCCESS;
  }

} 
