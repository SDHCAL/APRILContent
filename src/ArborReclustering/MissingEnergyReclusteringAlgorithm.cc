/// \file MissingEnergyReclusteringAlgorithm.cc
/*
 *
 * MissingEnergyReclusteringAlgorithm.cc source template automatically generated by a class generator
 * Creation date : mer. f�vr. 24 2016
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

#define __DEBUG__


#include "ArborReclustering/MissingEnergyReclusteringAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"

#include "ArborApi/ArborContentApi.h"
#include "ArborHelpers/ReclusterHelper.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborHelpers/SortingHelper.h"
#include "ArborHelpers/BDTBasedClusterIdHelper.h"
#include "ArborHelpers/TrackClusterPrintHelper.h"

namespace arbor_content
{

  pandora::StatusCode MissingEnergyReclusteringAlgorithm::Run()
  {
	// FIXME
	m_clusterName = "PrimaryTrees";
	m_photonClusterName = "PhotonClusters";

#if 0
	// Do we really need this recalculating ? optional ?
    // start by recalculating track-cluster association
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this, m_trackClusterAssociationAlgName));
#endif

	//////
	PrepareClusters();

	/////
	//ReclusterCluster();

	/////////////// do we need to check the of each track ? the unassociated track ?

    return pandora::STATUS_CODE_SUCCESS;
  }

  //////////////////////////////////////////////
  pandora::StatusCode MissingEnergyReclusteringAlgorithm::PrepareClusters()
  {
    // get current cluster list
    m_pClusterList = NULL;
	
	if (pandora::STATUS_CODE_SUCCESS != PandoraContentApi::GetList(*this, m_clusterName, m_pClusterList))
	{
#ifdef __DEBUG__
		std::cout << m_clusterName << " is not found." << std::endl;
#endif
	}
	else
	{
#ifdef __DEBUG__
		std::cout << m_clusterName << " is found." << std::endl;
#endif
	}

	// we include the photon clusters in reclustering, since there is a probability that hadronic
	// cluster is identified as photon cluster
    m_pPhotonClusterList = NULL;

    if (pandora::STATUS_CODE_SUCCESS != PandoraContentApi::GetList(*this, m_photonClusterName, m_pPhotonClusterList)) 
	{
#ifdef __DEBUG__
		std::cout << m_photonClusterName << " is not found." << std::endl;
#endif
	} 
	else 
	{
#ifdef __DEBUG__
		std::cout << m_photonClusterName << " is found." << std::endl;
#endif
	}

#ifdef __DEBUG__ 
    if(m_pClusterList !=NULL) TrackClusterPrintHelper::PrintCluster(m_pClusterList);
	if(m_pPhotonClusterList != NULL) TrackClusterPrintHelper::PrintCluster(m_pPhotonClusterList);
#endif

	m_clusterVector.clear();

	// FIXME
    if(m_pClusterList !=NULL && !m_pClusterList->empty()) 
	{
		m_clusterVector.insert(m_clusterVector.end(), m_pClusterList->begin(), m_pClusterList->end());

#ifdef __DEBUG__ 
		std::cout << "cluster size: " << m_pClusterList->size() << ", clusterVector size: " << m_clusterVector.size() << std::endl;
#endif
	}

	// FIXME
    if(m_pPhotonClusterList != NULL && !m_pPhotonClusterList->empty()) 
	{ 
		m_clusterVector.insert(m_clusterVector.end(), m_pPhotonClusterList->begin(), m_pPhotonClusterList->end());

#ifdef __DEBUG__ 
		std::cout << "photon cluster size: " << m_pPhotonClusterList->size() << ", clusterVector size: " 
			      << m_clusterVector.size() << std::endl;
#endif
	}

	//// sort 
    std::sort(m_clusterVector.begin(), m_clusterVector.end(), SortingHelper::SortByTrackClusterCompatibility(&this->GetPandora()));
	std::reverse(m_clusterVector.begin(), m_clusterVector.end());

#ifdef __DEBUG__
	std::cout << "-----------------> the sorted cluster:" << std::endl;

    for(unsigned int i=0 ; i<m_clusterVector.size() ; ++i)
    {
      const pandora::Cluster *const pCluster = m_clusterVector[i];
	  std::cout << "---energy of cluster: " << pCluster->GetElectromagneticEnergy() << std::endl;


      const pandora::TrackList trackList(pCluster->GetAssociatedTrackList());
      for (pandora::TrackList::const_iterator trackIter = trackList.begin(), trackIterEnd = trackList.end(); trackIter != trackIterEnd; ++trackIter) {
		  std::cout << "  ->track energy: " << (*trackIter)->GetEnergyAtDca() << std::endl;
	  }
	}
#endif

	// m_photonList is the recorder of all clusters in the photon cluster list
	// maybe just using pPhotonClusterList is OK ...
	m_photonList.insert(m_pPhotonClusterList->begin(), m_pPhotonClusterList->end());

	// move all clusters in the photon cluster list into the cluster list
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_INITIALIZED, !=, 
	       		                  PandoraContentApi::SaveList(*this, m_photonClusterName, m_clusterName, m_photonList));

	return pandora::STATUS_CODE_SUCCESS;
  }

  pandora::StatusCode MissingEnergyReclusteringAlgorithm::ReclusterCluster()
  {
	/////////////// check each cluster
    for(unsigned int i=0 ; i<m_clusterVector.size() ; ++i)
    {
      const pandora::Cluster *const pCluster = m_clusterVector[i];

	  if( NULL == pCluster || !ShouldReclusterCluster(pCluster) ) continue;

      // the clusters and tracks for reclustering
	
      pandora::ClusterList reclusterClusterList;
      reclusterClusterList.insert(pCluster);
      pandora::TrackList reclusterTrackList(pCluster->GetAssociatedTrackList());
#if 0

      UIntVector originalClusterIndices(1, i);

      pandora::ClusterFitResult parentFitResult;
      pandora::CartesianVector parentCentroid(0.f, 0.f, 0.f);
      pandora::ClusterFitHelper::FitFullCluster(pCluster, parentFitResult);

	  pandora::CartesianVector clusterInitialDirection = pCluster->GetInitialDirection();
      const pandora::StatusCode centroidStatusCode(ClusterHelper::GetCentroid(pCluster, parentCentroid));
#endif


	  //////
	  SearchNearbyClusters(pCluster, reclusterClusterList);

	  /////
	  ExecuteReclustering(pCluster, reclusterClusterList);

	  /////
	  ReAssociateTrackCluster();

#if 1
	  ////
	  TidyClusters();
#endif

    }

#if 0
	// put photon clusters back from clusters
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_INITIALIZED, !=, 
	        		                PandoraContentApi::SaveList(*this, m_clusterName, m_photonClusterName, m_photonList));

#endif

#if 0
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
	std::cout << "******* print cluster..." << std::endl;
	TrackClusterPrintHelper::PrintCluster(pClusterList);
#endif

	return pandora::STATUS_CODE_SUCCESS;
  }

  bool MissingEnergyReclusteringAlgorithm::ShouldReclusterCluster(const pandora::Cluster *const pCluster)
  {
      const pandora::TrackList trackList(pCluster->GetAssociatedTrackList());
      unsigned int nTrackAssociations(trackList.size());

      if((nTrackAssociations < m_minTrackAssociations) || (nTrackAssociations > m_maxTrackAssociations)) return false;

      float trackEnergySum(0.);

      for (pandora::TrackList::const_iterator trackIter = trackList.begin(), trackIterEnd = trackList.end(); 
	      trackIter != trackIterEnd; ++trackIter)
	  {
        trackEnergySum += (*trackIter)->GetEnergyAtDca();
	  }

      if(trackEnergySum < m_minTrackMomentum) return false;

      const float chi(ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), pCluster, trackList));

#ifdef __DEBUG__
	  std::cout << "chi: " << chi << ", minChi2: " << m_minChi2ToRunReclustering << ", trackEnergySum: " << trackEnergySum 
		        << ", cluster energy: " << pCluster->GetElectromagneticEnergy() << std::endl;
#endif

      // check for chi2 and missing energy in charged cluster
      if(chi*chi < m_minChi2ToRunReclustering || chi > 0.f) return false;

      // check for clusters that leave the detector
      if(ClusterHelper::IsClusterLeavingDetector(this->GetPandora(), pCluster)) return false;

	  return true;
  }

  pandora::StatusCode MissingEnergyReclusteringAlgorithm::SearchNearbyClusters(const pandora::Cluster *const pCluster, 
		                                                                       pandora::ClusterList& reclusterClusterList)
  {
      reclusterClusterList.insert(pCluster);

      pandora::ClusterFitResult parentFitResult;
      pandora::CartesianVector parentCentroid(0.f, 0.f, 0.f);
      pandora::ClusterFitHelper::FitFullCluster(pCluster, parentFitResult);

	  pandora::CartesianVector clusterInitialDirection = pCluster->GetInitialDirection();
      const pandora::StatusCode centroidStatusCode(ClusterHelper::GetCentroid(pCluster, parentCentroid));

      //------------->>>>>> find nearby clusters to potentially merge-in
      for(unsigned int j=0 ; j<m_clusterVector.size() ; ++j)
      {
        const pandora::Cluster *const pOtherCluster = m_clusterVector[j];
        if((NULL == pOtherCluster) || (pCluster == pOtherCluster)) continue;

		float bdtVal(1000.);
		BDTBasedClusterIdHelper::BDTEvaluate(pOtherCluster, bdtVal);

		// if it is photon cluster which is identified by BDT
		//FIXME: cut for BDT
		if(bdtVal>0.2) continue;

		// only neutral cluster ???
        if( !pOtherCluster->GetAssociatedTrackList().empty() ) continue;

        float clusterHitsDistance = std::numeric_limits<float>::max();
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetClosestDistanceApproach(pOtherCluster, pCluster, clusterHitsDistance));

        if(clusterHitsDistance < m_maxClusterHitsDistance)
        {
          reclusterClusterList.insert(pOtherCluster);
		  // FIXME
          //originalClusterIndices.push_back(j);
        }
        else
        {
			//FIXME
          if(clusterHitsDistance > 1000.) continue;

          if(pandora::STATUS_CODE_SUCCESS != centroidStatusCode || !parentFitResult.IsFitSuccessful()) continue;

          pandora::CartesianVector daughterCentroid(0.f, 0.f, 0.f);
          if(pandora::STATUS_CODE_SUCCESS != ClusterHelper::GetCentroid(pOtherCluster, daughterCentroid)) continue;

		  const float angle2( clusterInitialDirection.GetOpeningAngle( daughterCentroid - parentCentroid ) );
          if( angle2 > m_maxNeighborClusterAngle ) continue;

          reclusterClusterList.insert(pOtherCluster);
		  // FIXME
          //originalClusterIndices.push_back(j);
        }
      }

      //if(1 == reclusterClusterList.size()) continue;

	return pandora::STATUS_CODE_SUCCESS;
  }

  pandora::StatusCode MissingEnergyReclusteringAlgorithm::ExecuteReclustering(const pandora::Cluster *const pCluster, 
		  pandora::ClusterList& reclusterClusterList)
  {
	  //------------>>>>>> do reclustering 
	  // FIXME:: need to get chi ...
      //float bestChi(chi);
      //float bestChi(0.);

      pandora::TrackList reclusterTrackList(pCluster->GetAssociatedTrackList());

      // initialize reclustering
      std::string originalClusterListName;
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::InitializeReclustering(*this,
      reclusterTrackList, reclusterClusterList, originalClusterListName));

      m_bestReclusterClusterListName = originalClusterListName;

      for(pandora::StringVector::const_iterator clusteringAlgIter = m_clusteringAlgorithmList.begin(), 
	  endClusteringAlgIter = m_clusteringAlgorithmList.end() ; endClusteringAlgIter != clusteringAlgIter ; ++clusteringAlgIter)
      {

		/////
        const pandora::ClusterList *pReclusterClusterList = NULL;
        std::string reclusterClusterListName;
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::RunReclusteringAlgorithm(*this,
            *clusteringAlgIter, pReclusterClusterList, reclusterClusterListName));

        if(pReclusterClusterList->empty()) continue;

		/////
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this, m_associationAlgorithmName));

        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::PostRunReclusteringAlgorithm(*this, reclusterClusterListName));

        // run monitoring algorithm if provided
        if(!m_monitoringAlgorithmName.empty())
        {
          PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this,
              m_monitoringAlgorithmName));
        }
      
		// FIXME::
		if(IsBestReclustering(reclusterClusterListName)) m_bestReclusterClusterListName = reclusterClusterListName;

      }// for each algorithm

  	  PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::EndReclustering(*this, m_bestReclusterClusterListName));

	return pandora::STATUS_CODE_SUCCESS;
  }

  bool MissingEnergyReclusteringAlgorithm::IsBestReclustering(const std::string reclusterClusterListName)
  {
        const pandora::ClusterList *pReclusterClusterList = NULL;
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, reclusterClusterListName, pReclusterClusterList));

        for(pandora::ClusterList::const_iterator clusterIter = pReclusterClusterList->begin(), clusterEndIter = pReclusterClusterList->end() ;
            clusterEndIter != clusterIter ; ++ clusterIter)
        {
          const pandora::Cluster *const pReclusterCluster = *clusterIter;

          if( pReclusterCluster->GetAssociatedTrackList().empty() ) continue;

          //if( pReclusterCluster->GetAssociatedTrackList().size() != trackList.size() ) continue;

          pandora::ClusterList reclusterList;
          reclusterList.insert(pReclusterCluster);

          ReclusterResult reclusterResult;
          if(pandora::STATUS_CODE_SUCCESS != ReclusterHelper::ExtractReclusterResults(this->GetPandora(), *pReclusterClusterList, reclusterResult))
            continue;

          const float newChi(reclusterResult.GetChi());
          const float newChi2(reclusterResult.GetChi2());

          if(newChi2 < m_bestChi*m_bestChi)
          {
            m_bestChi = newChi;

            if(newChi2 < m_maxChi2ToStopReclustering)
              break;
          }

          break;
        }
		//------->>>> record the best reclustering

		// FIXME
		return true;
		//return trueorfalse;
  }

  pandora::StatusCode MissingEnergyReclusteringAlgorithm::ReAssociateTrackCluster()
  {
      //-------------> Recreate track-cluster associations for chosen recluster candidates
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::TemporarilyReplaceCurrentList<pandora::Cluster>(*this, m_bestReclusterClusterListName));
	  /////// >>>>>> ??????????? is this correct ??????????
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this, m_trackClusterAssociationAlgName));

    return pandora::STATUS_CODE_SUCCESS;
  }

#if 1
  pandora::StatusCode MissingEnergyReclusteringAlgorithm::TidyClusters()
  {
      // ------>>>>> tidy the cluster vector used for reclustering
      if( originalClusterListName != m_bestReclusterClusterListName )
      {
        // remove clusters used in reclustering from the list
        for(UIntVector::const_iterator iter = originalClusterIndices.begin(), endIter = originalClusterIndices.end() ;
            endIter != iter ; ++iter) 
		{
		  pandora::ClusterList::const_iterator cluIter = m_photonList.find(m_clusterVector[*iter]);
		  if(cluIter != m_photonList.end())
		  {
			  m_photonList.erase(cluIter);
		  }

          m_clusterVector[*iter] = NULL;
		}

        // add the newly created clusters to the list
        const pandora::ClusterList *pReclusterClusterList = NULL;
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, bestReclusterClusterListName, pReclusterClusterList));


        for(pandora::ClusterList::const_iterator clusterIter = pReclusterClusterList->begin(), clusterEndIter = pReclusterClusterList->end() ;
            clusterEndIter != clusterIter ; ++ clusterIter)
        {
          const pandora::Cluster *const pReclusterCluster = *clusterIter;


          if(pReclusterCluster->GetAssociatedTrackList().empty()) continue;

          m_clusterVector.push_back(pReclusterCluster);
        }
      }

    return pandora::STATUS_CODE_SUCCESS;
  }


#endif


  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode MissingEnergyReclusteringAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_minTrackAssociations = 1;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinTrackAssociations", m_minTrackAssociations));

    m_maxTrackAssociations = 1;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTrackAssociations", m_maxTrackAssociations));

    m_minChi2ToRunReclustering = 1.8f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinChi2ToRunReclustering", m_minChi2ToRunReclustering));

    m_maxChi2ToStopReclustering = 0.5f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxChi2ToStopReclustering", m_maxChi2ToStopReclustering));

    m_maxClusterHitsDistance = 70.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxClusterHitsDistance", m_maxClusterHitsDistance));

    m_maxNeighborClusterAngle = 0.5f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNeighborClusterAngle", m_maxNeighborClusterAngle));

    m_minTrackMomentum = 0.8f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinTrackMomentum", m_minTrackMomentum));

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ProcessAlgorithmList(*this, xmlHandle,
        "clusteringAlgorithms", m_clusteringAlgorithmList));

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ProcessAlgorithm(*this, xmlHandle,
        "ClusterAssociation", m_associationAlgorithmName));

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ProcessAlgorithm(*this, xmlHandle,
        "TrackClusterAssociation", m_trackClusterAssociationAlgName));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ProcessAlgorithm(*this, xmlHandle,
        "Monitoring", m_monitoringAlgorithmName));

    return pandora::STATUS_CODE_SUCCESS;
  }


} 
