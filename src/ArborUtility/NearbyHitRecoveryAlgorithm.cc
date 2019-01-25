/**
 *  @file   NearbyHitRecoveryAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating cluster cleaning algorithm class
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborUtility/NearbyHitRecoveryAlgorithm.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborHelpers/CaloHitRangeSearchHelper.h"
#include "ArborHelpers/CaloHitNeighborSearchHelper.h"
#include "ArborHelpers/HistogramHelper.h"
#include "ArborUtility/EventPreparationAlgorithm.h"
#include "ArborApi/ArborContentApi.h"

namespace arbor_content
{

pandora::StatusCode NearbyHitRecoveryAlgorithm::Run()
{
	ClusterCaloHitListMap clusterCaloHitListMap;

	MakeClusterHitsAssociation(clusterCaloHitListMap);
	AddHitToCluster(clusterCaloHitListMap);
    
	return pandora::STATUS_CODE_SUCCESS;
}

pandora::StatusCode NearbyHitRecoveryAlgorithm::MakeClusterHitsAssociation(ClusterCaloHitListMap& clusterCaloHitListMap)
{
    const pandora::CaloHitList *pCaloHitList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

	int nUnclusteredHits = 0;

    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(); iter != pCaloHitList->end(); ++iter)
	{
		const pandora::CaloHit* const pCaloHit = *iter;

        if (PandoraContentApi::IsAvailable(*this, pCaloHit))
		{
		   ++nUnclusteredHits;
		   const pandora::MCParticle* pMCHitParticle = nullptr;

		   pandora::CartesianVector testPosition = pCaloHit->GetPositionVector();
		   
		   const int nNeighbor = m_nNeighborHits;
		   pandora::CaloHitList neighborHits;

		   CaloHitNeighborSearchHelper::SearchNeighbourHits(testPosition, nNeighbor, neighborHits);

		   //std::cout << " calo hit is not clustered: " << pCaloHit << ", X = " << testPosition.GetX() << ", " << testPosition.GetY() 
		   //	<< ", " << testPosition.GetZ() << std::endl;

		   const pandora::Cluster* clusterToAdd = nullptr;
		   float hitsDistance = 0.;

		   for(auto& caloHit : neighborHits)
		   {
			   auto& hitPos = caloHit->GetPositionVector();
               const arbor_content::CaloHit *const pArborCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(caloHit);
			   //std::cout << "     the nearby hit distance: " << (hitPos - testPosition).GetMagnitude() 
				 //  << ", pos: " << hitPos.GetX() << ", " << hitPos.GetY() << ", " << hitPos.GetZ() 
				   //<< ", cluster: " << pArborCaloHit->GetMother() << std::endl;

			   if(pArborCaloHit != nullptr && clusterToAdd == nullptr)
			   {
				   clusterToAdd = pArborCaloHit->GetMother();
				   hitsDistance = (hitPos - testPosition).GetMagnitude();
			   }
		   }

           try
           {
           	 pMCHitParticle = pandora::MCParticleHelper::GetMainMCParticle(pCaloHit);
           	//std::cout << "calo hit: " << caloHit << ", mcp: " << pMCHitParticle << std::endl;
           }
           catch (pandora::StatusCodeException &)
           {
		       continue;
           }

		   if(clusterToAdd != nullptr && hitsDistance < m_maxHitsDistance)
		   {
		      const pandora::MCParticle* pClusterMCParticle  = nullptr;

              try
              {
              	 pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(clusterToAdd);
              }
              catch (pandora::StatusCodeException &)
              {
		          continue;
		      }
		      //////
		   	
		      //std::cout << "  ====== hit: " << pCaloHit << ", mcp: " << pMCHitParticle 
		   	  // << ", the cluster which may merge the hit: " << clusterToAdd << ", mcp :" << pClusterMCParticle << endl;

		      if(clusterCaloHitListMap.find( clusterToAdd ) == clusterCaloHitListMap.end())
		      {
		          pandora::CaloHitList hitList;
		          hitList.push_back( pCaloHit );
		          clusterCaloHitListMap[clusterToAdd] = hitList;
		      }
		      else
		      {
		          clusterCaloHitListMap[clusterToAdd].push_back( pCaloHit );
		      }
    	
			  int hitMCPCharge = pandora::PdgTable::GetParticleCharge(pMCHitParticle->GetParticleId());
			  int clusterMCPCharge = pandora::PdgTable::GetParticleCharge(pClusterMCParticle->GetParticleId());

	          std::vector<float> vars;
	          vars.push_back( float(EventPreparationAlgorithm::GetEventNumber()) );
	          vars.push_back( hitsDistance );
	          vars.push_back( float(pMCHitParticle == pClusterMCParticle) );
	          vars.push_back( float(hitMCPCharge) );
	          vars.push_back( float(clusterMCPCharge) );

		      HistogramManager::CreateFill("TestAddHitToCluster", 
			  "evtNumber:hitsDistance:isRight:hitMCPCharge:clusterMCPCharge", vars);
		   }
		}
	}

	std::cout << "===unClusteredHits size: " << nUnclusteredHits << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
}

pandora::StatusCode NearbyHitRecoveryAlgorithm::AddHitToCluster(ClusterCaloHitListMap& clusterCaloHitListMap)
{
	// add the unused hit to exsiting clusters (not by MCP)
	for(auto clusterIt = clusterCaloHitListMap.begin(); clusterIt != clusterCaloHitListMap.end(); ++clusterIt)
	{
		const auto& cluster = clusterIt->first;
		const auto& caloHitList = clusterIt->second;

		if(caloHitList.empty()) continue;

		const pandora::CaloHitList& hitsAddToCluster = caloHitList;

        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::AddToCluster(*this, cluster, &hitsAddToCluster));
	}

	// check 
    const pandora::CaloHitList *pCaloHitList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));
	//std::cout << "------- # CaloHit : " << pCaloHitList->size() << std::endl;
	
	pandora::CaloHitList unClusteredHits;

    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(); iter != pCaloHitList->end(); ++iter)
	{
		const pandora::CaloHit* const pCaloHit = *iter;

        if (PandoraContentApi::IsAvailable(*this, pCaloHit))
		{
			unClusteredHits.push_back(pCaloHit);
		}
	}

	std::cout << "===unClusteredHits size: " << unClusteredHits.size() << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
}
	
//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode NearbyHitRecoveryAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
    m_nNeighborHits = 10;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "NeighborHitsNumber", m_nNeighborHits));

    m_maxHitsDistance = std::numeric_limits<float>::max();
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxHitsDistance", m_maxHitsDistance));

    return pandora::STATUS_CODE_SUCCESS;
}

} // namespace arbor_content