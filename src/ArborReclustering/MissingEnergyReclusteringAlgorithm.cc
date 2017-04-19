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


#include "ArborReclustering/MissingEnergyReclusteringAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"

#include "ArborApi/ArborContentApi.h"
#include "ArborHelpers/ReclusterHelper.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborHelpers/SortingHelper.h"

namespace arbor_content
{

pandora::StatusCode MissingEnergyReclusteringAlgorithm::Run()
{
    // start by recalculating track-cluster association
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this, m_trackClusterAssociationAlgName));

    // get current cluster list
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    // FIXME:: maybe we should also include the photon clusters
    const pandora::ClusterList *pPhotonClusterList = NULL;
    std::string clusterName = "PhotonClusters";

    if (pandora::STATUS_CODE_SUCCESS != PandoraContentApi::GetList(*this, clusterName, pPhotonClusterList))
    {
        std::cout << clusterName << " is not found." << std::endl;
    }
    else
    {
        std::cout << clusterName << " is found." << std::endl;
    }

    if(pClusterList->empty())
        return pandora::STATUS_CODE_SUCCESS;

    // loop start from here
    // the condition of loop ending is: each cluster in the improper track-cluster associations has no nearby cluster
    bool canLoop(true);

    while(canLoop)
    {
		canLoop = false;

        //std::cout << "cluster size: " << pClusterList->size() << ", photon cluster size: " << pPhotonClusterList->size() << std::endl;

        pandora::ClusterVector clusterVector(pClusterList->begin(), pClusterList->end());
        clusterVector.insert(clusterVector.end(), pPhotonClusterList->begin(), pPhotonClusterList->end());

        std::sort(clusterVector.begin(), clusterVector.end(), SortingHelper::SortByTrackClusterCompatibility(&this->GetPandora()));

        pandora::ClusterList photonClusters;
        photonClusters.insert(pPhotonClusterList->begin(), pPhotonClusterList->end());

        std::string treeName("PrimaryTrees");
        std::string photonName("PhotonClusters");

        // move the photon clusters to the clusters list
        PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_INITIALIZED, !=,
                                        PandoraContentApi::SaveList(*this, photonName, treeName, photonClusters));

        for(unsigned int i=0 ; i<clusterVector.size() ; ++i)
        {
            const pandora::Cluster *const pCluster = clusterVector[i];

            if( NULL == pCluster )
                continue;

            const pandora::TrackList trackList(pCluster->GetAssociatedTrackList());
            unsigned int nTrackAssociations(trackList.size());

            if((nTrackAssociations < m_minTrackAssociations) || (nTrackAssociations > m_maxTrackAssociations))
                continue;

            float trackEnergySum(0.);

            for (pandora::TrackList::const_iterator trackIter = trackList.begin(), trackIterEnd = trackList.end(); trackIter != trackIterEnd; ++trackIter)
                trackEnergySum += (*trackIter)->GetEnergyAtDca();

            if(trackEnergySum < m_minTrackMomentum)
                continue;

            const float chi(ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), pCluster, trackList));

            // check for chi2 and missing energy in charged cluster
            if(chi*chi < m_minChi2ToRunReclustering || chi > 0.f)
                continue;

            // check for clusters that leave the detector
            if(ClusterHelper::IsClusterLeavingDetector(this->GetPandora(), pCluster))
                continue;

            //std::cout << "chi: " << chi << ", trackEnergySum: " << trackEnergySum
            //        << ", cluster energy: " << pCluster->GetElectromagneticEnergy() << std::endl;

            // prepare clusters and tracks for reclustering
            pandora::ClusterList reclusterClusterList;
            reclusterClusterList.insert(pCluster);
            pandora::TrackList reclusterTrackList(trackList);

            UIntVector originalClusterIndices(1, i);

            pandora::ClusterFitResult parentFitResult;
            pandora::CartesianVector parentCentroid(0.f, 0.f, 0.f);
            pandora::ClusterFitHelper::FitFullCluster(pCluster, parentFitResult);

            const pandora::StatusCode centroidStatusCode(ClusterHelper::GetCentroid(pCluster, parentCentroid));

            // find nearby clusters to potentially merge-in
            for(unsigned int j=0 ; j<clusterVector.size() ; ++j)
            {
                const pandora::Cluster *const pOtherCluster = clusterVector[j];

                if((NULL == pOtherCluster) || (pCluster == pOtherCluster))
                    continue;

                if( ! pOtherCluster->GetAssociatedTrackList().empty() )
                    continue;

                float clusterHitsDistance = std::numeric_limits<float>::max();
                PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetClosestDistanceApproach(pOtherCluster, pCluster, clusterHitsDistance));

                if(clusterHitsDistance < m_maxClusterHitsDistance)
                {
                    reclusterClusterList.insert(pOtherCluster);
                    originalClusterIndices.push_back(j);
                }
                else
                {
                    if(pandora::STATUS_CODE_SUCCESS != centroidStatusCode || !parentFitResult.IsFitSuccessful())
                        continue;

                    pandora::CartesianVector daughterCentroid(0.f, 0.f, 0.f);

                    if(pandora::STATUS_CODE_SUCCESS != ClusterHelper::GetCentroid(pOtherCluster, daughterCentroid))
                        continue;

                    const float angle( parentFitResult.GetDirection().GetOpeningAngle( daughterCentroid - parentCentroid ) );

                    if( angle > m_maxNeighborClusterAngle )
                        continue;

                    reclusterClusterList.insert(pOtherCluster);
                    originalClusterIndices.push_back(j);
                }
            }

            //std::cout << "find the nearby cluster... size: " << reclusterClusterList.size() << std::endl;

            if(1 == reclusterClusterList.size())
            {
                continue;
            }

            // initialize reclustering
            std::string originalClusterListName;
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::InitializeReclustering(*this,
                                     reclusterTrackList, reclusterClusterList, originalClusterListName));

            float bestChi(chi);
            std::string bestReclusterClusterListName(originalClusterListName);

            for(pandora::StringVector::const_iterator clusteringAlgIter = m_clusteringAlgorithmList.begin(), endClusteringAlgIter = m_clusteringAlgorithmList.end() ;
                    endClusteringAlgIter != clusteringAlgIter ; ++clusteringAlgIter)
            {

                const pandora::ClusterList *pReclusterClusterList = NULL;
                std::string reclusterClusterListName;
                PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::RunReclusteringAlgorithm(*this,
                                         *clusteringAlgIter, pReclusterClusterList, reclusterClusterListName));

                //std::cout << "pReclusterClusterList: " << pReclusterClusterList->size() << std::endl;
                if(pReclusterClusterList->empty())
                    continue;

                PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this, m_associationAlgorithmName));
                PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::PostRunReclusteringAlgorithm(*this, reclusterClusterListName));


                // run monitoring algorithm if provided

                if(!m_monitoringAlgorithmName.empty())
                {
                    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this,
                                             m_monitoringAlgorithmName));
                }


                // find the original tracks that triggered the reclustering
                for(pandora::ClusterList::const_iterator clusterIter = pReclusterClusterList->begin(), clusterEndIter = pReclusterClusterList->end() ;
                        clusterEndIter != clusterIter ; ++ clusterIter)
                {
                    const pandora::Cluster *const pReclusterCluster = *clusterIter;

                    //std::cout << "recluster: energy: " << pReclusterCluster->GetElectromagneticEnergy() << std::endl;

                    if( pReclusterCluster->GetAssociatedTrackList().empty() )
                        continue;

                    if( pReclusterCluster->GetAssociatedTrackList().size() != trackList.size() )
                        continue;

                    pandora::ClusterList reclusterList;
                    reclusterList.insert(pReclusterCluster);

                    ReclusterResult reclusterResult;
                    if(pandora::STATUS_CODE_SUCCESS != ReclusterHelper::ExtractReclusterResults(this->GetPandora(), *pReclusterClusterList, reclusterResult))
                        continue;

                    const float newChi(reclusterResult.GetChi());
                    const float newChi2(reclusterResult.GetChi2());

                    if(newChi2 < bestChi*bestChi)
                    {
                        bestChi = newChi;
                        bestReclusterClusterListName = reclusterClusterListName;

                        if(newChi2 < m_maxChi2ToStopReclustering)
                            break;
                    }

                    break;
                }
            }

            // Recreate track-cluster associations for chosen recluster candidates
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::TemporarilyReplaceCurrentList<pandora::Cluster>(*this, bestReclusterClusterListName));
            //std::cout << "m_trackClusterAssociationAlgName: " << m_trackClusterAssociationAlgName << std::endl;
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this, m_trackClusterAssociationAlgName));

            // tidy the cluster vector used for reclustering
            if( originalClusterListName != bestReclusterClusterListName )
            {
                // remove clusters used in reclustering from the list
                for(UIntVector::const_iterator iter = originalClusterIndices.begin(), endIter = originalClusterIndices.end() ;
                        endIter != iter ; ++iter)
                {
                    //std::cout << "the cluster to be removed: " << (clusterVector[*iter])->GetElectromagneticEnergy() << std::endl;

                    pandora::ClusterList::const_iterator cluIter = photonClusters.find(clusterVector[*iter]);
                    if(cluIter != photonClusters.end())
                    {
                        photonClusters.erase(cluIter);
                    }

                    clusterVector[*iter] = NULL;
                }

                std::cout << "clusterVector size:: " << clusterVector.size() << std::endl;

                // add the newly created clusters to the list
                const pandora::ClusterList *pReclusterClusterList = NULL;
                PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, bestReclusterClusterListName, pReclusterClusterList));

                //std::cout << "pReclusterClusterList size: " << pReclusterClusterList->size() << std::endl;

                for(pandora::ClusterList::const_iterator clusterIter = pReclusterClusterList->begin(),
                        clusterEndIter = pReclusterClusterList->end() ; clusterEndIter != clusterIter ; ++ clusterIter)
                {
                    const pandora::Cluster *const pReclusterCluster = *clusterIter;

                    if(!pReclusterCluster->GetAssociatedTrackList().empty())
                        continue;

                    clusterVector.push_back(pReclusterCluster);
                }

				canLoop = true;
            }
			else
			{
				//std::cout << "no reclustering..." << std::endl;
			}

            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::EndReclustering(*this, bestReclusterClusterListName));
        }

        // put photon clusters back from clusters
        PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_INITIALIZED, !=,
                                        PandoraContentApi::SaveList(*this, treeName, photonName, photonClusters));
    } // loop ends

    //std::cout << "pClusterList: " << pClusterList->size() << std::endl;
    //std::cout << "pPhotonClusterList: " << pPhotonClusterList->size() << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
}

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

