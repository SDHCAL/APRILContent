/// \file TrackDrivenSeedingTool.cc
/*
 *
 * TrackDrivenSeedingTool.cc source template automatically generated by a class generator
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


#include "ArborTools/TrackDrivenSeedingTool.h"

#include "ArborHelpers/GeometryHelper.h"
#include "ArborHelpers/SortingHelper.h"

#include "Pandora/AlgorithmHeaders.h"
#include "Pandora/Algorithm.h"

#include "ArborObjects/CaloHit.h"
#include "ArborObjects/Connector.h"
#include "ArborHelpers/CaloHitHelper.h"
#include "ArborHelpers/CaloHitRangeSearchHelper.h"
#include "ArborHelpers/CaloHitNeighborSearchHelper.h"

namespace arbor_content
{
  TrackCaloHitVector TrackDrivenSeedingTool::m_trackHitVector;

  pandora::StatusCode TrackDrivenSeedingTool::Process(const pandora::Algorithm &algorithm, const pandora::CaloHitList *const pCaloHitList)
  {
    // get current track list
    const pandora::TrackList *pTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(algorithm, pTrackList));      
    //std::cout << "pTrackList: " << pTrackList->size() << std::endl;

    if(pTrackList->empty())
      return pandora::STATUS_CODE_SUCCESS;

    // ordered calo hit list
    pandora::OrderedCaloHitList* pOrderedCaloHitList = CaloHitRangeSearchHelper::GetOrderedEcalCaloHitList();

	//std::cout << "ptr: " << pOrderedCaloHitList << ", size: " << pOrderedCaloHitList->size() << std::endl;
	pandora::OrderedCaloHitList& orderedCaloHitList = *pOrderedCaloHitList;

	m_trackHitVector.clear();

    //clock_t t0, t1;
	//t0 = clock();
	
    for(pandora::TrackList::const_iterator trackIter = pTrackList->begin(), trackEndIter = pTrackList->end() ;
        trackEndIter != trackIter ; ++trackIter)
    {
      const pandora::Track *pTrack = *trackIter;

      if(!pTrack->ReachesCalorimeter() || !pTrack->CanFormPfo())
        continue;

	  //if(fabs(pTrack->GetMomentumAtDca().GetMagnitude() - 5.15) > 0.01) continue;

      pandora::CaloHitVector caloHitVector;
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->FindInitialCaloHits(algorithm, pTrack, orderedCaloHitList, caloHitVector));

	  // TODO
	  // make relationship between track and calohitvector
	  // it can be used for further track cluster association 
	  // however it should be careful that the same calo hit may enter into different caloHitVector 
	  // due to nearby tracks
	  // or nearby neutral hit may enter into this connection

	  // sort by layer
      std::sort(caloHitVector.begin(), caloHitVector.end(), SortingHelper::SortCaloHitsByLayer);

	  m_trackHitVector[pTrack] = caloHitVector;

      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->ConnectCaloHits(algorithm, pTrack, caloHitVector));
    }

	//t1 = clock();
	//std::cout << " time : " << t1 - t0 << std::endl;
	
    //CheckInitialTrackHit();
	
	CleanTrackInitHitsAssociation(algorithm);

    return pandora::STATUS_CODE_SUCCESS;
  }

  void TrackDrivenSeedingTool::CheckInitialTrackHit() const
  {
	  std::cout << "TrackDrivenSeedingTool::CheckInitialTrackHit" << std::endl;

      for(auto trackHitsIter = m_trackHitVector.begin(); trackHitsIter != m_trackHitVector.end(); ++trackHitsIter)
      {
          auto track = trackHitsIter->first;
          auto& caloHits = trackHitsIter->second;
          
          std::cout << "track p : " << track->GetMomentumAtDca().GetMagnitude() << ", hits size: " << caloHits.size() << std::endl;
          
          const pandora::MCParticle *pTrackMCParticle = nullptr;
          const pandora::MCParticle *pCaloHitMCParticle = nullptr;
          
          try
          {
			  pTrackMCParticle = pandora::MCParticleHelper::GetMainMCParticle( track );
			  //std::cout << "track MCP : " << pTrackMCParticle << std::endl;
          }
          catch (pandora::StatusCodeException &)
          {
          }
          
          //
          for(unsigned int iHit = 0; iHit < caloHits.size(); ++iHit)
          {
			  std::cout << " ******** hit: " << caloHits.at(iHit) << std::endl;
              try
              {
				  pCaloHitMCParticle = pandora::MCParticleHelper::GetMainMCParticle( caloHits.at(iHit) );
              
                  if(pCaloHitMCParticle != pTrackMCParticle )
                  {
                      std::cout << " *** WARNING *** hit - track diff MCP" << std::endl;
                      //std::cout << "calo hit MCP : " << pCaloHitMCParticle << std::endl;
                      std::cout << "calo hit : " << caloHits.at(iHit) << ", track: " << track << std::endl;
                      
                      const pandora::CartesianVector & pos = track->GetTrackStateAtCalorimeter().GetPosition();
                      
                      std::cout << "track on calo pos: " << pos.GetX() << ", " << pos.GetY() << ", " << pos.GetZ() << std::endl;
                  }
          
              }
              catch (pandora::StatusCodeException &)
              {
              }
          }
          
          std::cout << " ============================= " << std::endl;
      }
  }


  void TrackDrivenSeedingTool::CleanTrackInitHitsAssociation(const pandora::Algorithm &algorithm) const
  {
	  // create a map from hit to track
      std::map< const pandora::CaloHit*, std::list<const pandora::Track*> > caloHitTracks;
      
      for(auto trackHitsIter = m_trackHitVector.begin(); trackHitsIter != m_trackHitVector.end(); ++trackHitsIter)
      {
          auto track = trackHitsIter->first;
          auto& caloHits = trackHitsIter->second;
          
          for(unsigned int iHit = 0; iHit < caloHits.size(); ++iHit)
          {
              auto pCaloHit = caloHits.at(iHit);
              
              if(caloHitTracks.find(pCaloHit) == caloHitTracks.end())
              {
                  std::list<const pandora::Track*> trackList;
                  trackList.push_back(track);
                  caloHitTracks[pCaloHit] = trackList;
              }
              else
              {
                  caloHitTracks[pCaloHit].push_back(track);
              }
          }
      }
      
	  // check each calo hit:
	  // if a hit mapped to more than one track
	  // then this mapping should be corrected
      for(auto hitIter = caloHitTracks.begin(); hitIter != caloHitTracks.end(); ++hitIter)
      {
		  auto caloHit = hitIter->first;
          auto tracks = hitIter->second;

          if(tracks.size() > 1)
          {
#if 0
              std::cout << "warning: track size > 1, calo hit: " << hitIter->first << std::endl;

              for(auto trackIter = tracks.begin(); trackIter != tracks.end(); ++trackIter)
              {
                  std::cout << " ---> track: " << *trackIter << ", p = " << (*trackIter)->GetMomentumAtDca().GetMagnitude() 
					  << ", GetEndCap: " << (*trackIter)->IsProjectedToEndCap() << std::endl;
              }
#endif

			  // get the closest track to hit
			  const pandora::Track* bestTrackToHit = nullptr;
			  float bestTrackHitDistance = 1.e6;

              for(auto trackIter = tracks.begin(); trackIter != tracks.end(); ++trackIter)
			  {
				  auto testingTrack = *trackIter;

                  pandora::CartesianVector projectionOnHelix(0.f, 0.f, 0.f);
    
				  const float bField(PandoraContentApi::GetPlugins(algorithm)->GetBFieldPlugin()->GetBField(
							  pandora::CartesianVector(0.f, 0.f, 0.f)));

				  const pandora::Helix helix(testingTrack->GetTrackStateAtCalorimeter().GetPosition(),
						  testingTrack->GetTrackStateAtCalorimeter().GetMomentum(), testingTrack->GetCharge(), bField);

                  if(pandora::STATUS_CODE_SUCCESS != GeometryHelper::GetProjectionOnHelix(helix, caloHit->GetPositionVector(), projectionOnHelix))
                    continue;

			      float trackHitDistance = (projectionOnHelix - caloHit->GetPositionVector()).GetMagnitude();

				  if(trackHitDistance < bestTrackHitDistance)
				  {
					  bestTrackToHit = testingTrack;
					  bestTrackHitDistance = trackHitDistance;
				  }
			  }

			  // remove the hit which badly associated to track
              for(auto trackIter = tracks.begin(); trackIter != tracks.end(); ++trackIter)
              {
				  auto testingTrack = *trackIter;
                  auto trackIterInMap = m_trackHitVector.find(testingTrack);

				  if(trackIterInMap == m_trackHitVector.end() ) continue;

				  auto suspiciousTrack = trackIterInMap->first;

				  if( (suspiciousTrack != bestTrackToHit) && (!IsSiblingTrack(bestTrackToHit, suspiciousTrack)) ) 
				  {
                     auto& associatedCaloHits = trackIterInMap->second;
					 auto foundCaloHit = std::find(associatedCaloHits.begin(), associatedCaloHits.end(), caloHit);

					 if(foundCaloHit != associatedCaloHits.end())
					 {
#if 0
				         std::cout << "remove hit: " << *foundCaloHit << " from track: " << suspiciousTrack
							 << ", p = " << suspiciousTrack->GetMomentumAtDca().GetMagnitude() 
					         << ", GetEndCap: " << suspiciousTrack->IsProjectedToEndCap() << std::endl;
#endif
						 associatedCaloHits.erase(foundCaloHit);
					 }
				  }
			  } 

          } // if a hit is associated to more than one track

      } // for each init calo hit 
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
	bool TrackDrivenSeedingTool::IsSiblingTrack(const pandora::Track *const trackA, const pandora::Track *const trackB) const
	{
		auto& siblingTrackList = trackA->GetSiblingList();

		return ( std::find(siblingTrackList.begin(), siblingTrackList.end(), trackB) != siblingTrackList.end()  ? true : false) ;
	}

  //------------------------------------------------------------------------------------------------------------------------------------------
	const pandora::CartesianVector TrackDrivenSeedingTool::GetFromHitDirection(const arbor_content::CaloHit *const pCaloHit)
	{
        pandora::CartesianVector meanBackwardDirection(0, 0, 0);
		const int backwardReferenceDirectionDepth = 2;

		const ConnectorList &backwardConnectorList(ArborContentApi::GetConnectorList(pCaloHit, BACKWARD_DIRECTION));

		if(backwardConnectorList.size() > 0)
		{
			CaloHitHelper::GetMeanDirection(pCaloHit, BACKWARD_DIRECTION, meanBackwardDirection, backwardReferenceDirectionDepth);
		    return meanBackwardDirection;
		}
		else
		{
			//std::cout << " no connector, the hit position: " << pCaloHit->GetPositionVector() << std::endl;
			return pCaloHit->GetPositionVector();
		}

		//std::cout << " -------------------- " << std::endl;
	}

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode TrackDrivenSeedingTool::FindInitialCaloHits(const pandora::Algorithm &algorithm, const pandora::Track *pTrack, const pandora::OrderedCaloHitList& orderedCaloHitList,
      pandora::CaloHitVector &caloHitVector)
  {
    const float bField(PandoraContentApi::GetPlugins(algorithm)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));
    const pandora::Helix helix(pTrack->GetTrackStateAtCalorimeter().GetPosition(),
        pTrack->GetTrackStateAtCalorimeter().GetMomentum(), pTrack->GetCharge(), bField);

	for(auto orderedCaloHitListIter = orderedCaloHitList.begin(); orderedCaloHitListIter != orderedCaloHitList.end(); ++orderedCaloHitListIter)
	{
		int pseudoLayer = orderedCaloHitListIter->first;
        if(pseudoLayer > m_maxInitialPseudoLayer) break;

		const pandora::CaloHit* bestHit = nullptr;
		float bestDistance = 1.e6;
        
		float range = 100.;
		pandora::CaloHitList hitsInRange;
		pandora::CartesianVector trackPositionAtCalo = pTrack->GetTrackStateAtCalorimeter().GetPosition();

		CaloHitRangeSearchHelper::SearchHitsInLayer(trackPositionAtCalo, pseudoLayer, range, hitsInRange);

		for(auto hitIter = hitsInRange.begin(); hitIter != hitsInRange.end(); ++hitIter)
		{
            const pandora::CaloHit *const pCaloHit = *hitIter;

            if( ! PandoraContentApi::IsAvailable<pandora::CaloHit>( algorithm, pCaloHit ) )
              continue;

            if(!m_shouldUseIsolatedHits && pCaloHit->IsIsolated())
              continue;

            pandora::CartesianVector projectionOnHelix(0.f, 0.f, 0.f);

            if(pandora::STATUS_CODE_SUCCESS != GeometryHelper::GetProjectionOnHelix(helix, pCaloHit->GetPositionVector(), projectionOnHelix))
              continue;

			float trackHitDistance = (projectionOnHelix-pCaloHit->GetPositionVector()).GetMagnitude() ;

            if(trackHitDistance > m_maxInitialTrackDistance)
              continue;

			if(trackHitDistance < bestDistance)
			{
				bestHit = pCaloHit;
				bestDistance = trackHitDistance;
			}
		}

		if(bestHit != nullptr) caloHitVector.push_back(bestHit);
	}

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode TrackDrivenSeedingTool::ConnectCaloHits(const pandora::Algorithm &algorithm, const pandora::Track *pTrack,
      pandora::CaloHitVector &caloHitVector)
  {
    if(caloHitVector.empty())
      return pandora::STATUS_CODE_SUCCESS;


    // get b field and track helix
    const float bField(PandoraContentApi::GetPlugins(algorithm)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));
    const pandora::Helix helix(pTrack->GetTrackStateAtCalorimeter().GetPosition(),
        pTrack->GetTrackStateAtCalorimeter().GetMomentum(), pTrack->GetCharge(), bField);


    // ordered calo hit list
    pandora::OrderedCaloHitList* pOrderedCaloHitList = CaloHitRangeSearchHelper::GetOrderedCaloHitList();

	//std::cout << "ptr: " << pOrderedCaloHitList << std::endl;
	pandora::OrderedCaloHitList& orderedCaloHitList = *pOrderedCaloHitList;

	//int nInitHits = caloHitVector.size();

	//int firstLayer = -1;

	//std::cout << "Track seeding hits: " << nInitHits << std::endl;

	// loop for the starting hits, and make connector between starting hit and end hit, then the end
	// hit will be taken as starting hit, and added to the starting hits vector
	// i.e., caloHitVector is the the vector of starting hit, it is will be updated during the clustering
    for(size_t i=0 ; i<caloHitVector.size() ; i++)
    {
      const arbor_content::CaloHit *const pCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(caloHitVector.at(i));

      if(!m_shouldUseIsolatedHits && pCaloHit->IsIsolated())
        continue;

      const pandora::CartesianVector &position(pCaloHit->GetPositionVector());
      const unsigned int pseudoLayer = pCaloHit->GetPseudoLayer();

	  //if(i==0) firstLayer = pseudoLayer;

#if 0
	  if(i<=nInitHits)
	  {
		  std::cout << "seeding hit layer: " << pseudoLayer << ", current connector hit size: " << caloHitVector.size() << std::endl;
	  }
#endif

      pandora::CartesianVector helixProjection(0.f, 0.f, 0.f);

      if(pandora::STATUS_CODE_SUCCESS != GeometryHelper::GetProjectionOnHelix(helix, position, helixProjection))
        continue;

      const pandora::CartesianVector extrapolatedMomentum(helix.GetExtrapolatedMomentum(helixProjection));
      //const pandora::CartesianVector trackMomentum(pTrack->GetTrackStateAtCalorimeter().GetMomentum());

	  // the layer of the starting hit
      pandora::OrderedCaloHitList::const_iterator plIter = orderedCaloHitList.find(pseudoLayer);

      // unexpected ???
      if(orderedCaloHitList.end() == plIter || plIter->second->empty())
        continue;

      // start to next pseudo layer
      plIter++;

      if(orderedCaloHitList.end() == plIter || plIter->second->empty())
        continue;

      while(1)
      {
        if(plIter == orderedCaloHitList.end())
          break;

        if(plIter->first > pseudoLayer+m_maxConnectionPseudoLayer)
          break;

        if(!plIter->second->empty())
        {
          // create search range for each layer 

          const float range = 200.; // OK ???

		  // use pseudo layer
          int toPseudoLayer = plIter->first;
          pandora::CaloHitList hitsInRange;

          CaloHitRangeSearchHelper::SearchHitsInLayer(position, toPseudoLayer, range, hitsInRange);
	
          for(pandora::CaloHitList::const_iterator iter = hitsInRange.begin(), endIter = hitsInRange.end() ;
              endIter != iter ; ++iter)
          {
            const arbor_content::CaloHit *const pTestCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(*iter);

            if( ! PandoraContentApi::IsAvailable<pandora::CaloHit>( algorithm, pTestCaloHit ) )
              continue;

            // check for existing connection
            if(ArborContentApi::IsConnected(pCaloHit, pTestCaloHit, arbor_content::FORWARD_DIRECTION))
              continue;

            const pandora::Granularity &granularity(algorithm.GetPandora().GetGeometry()->GetHitTypeGranularity(pTestCaloHit->GetHitType()));
            const float maxTransverseDistance = (granularity >= pandora::COARSE) ? m_maxTransverseDistanceCoarse : m_maxTransverseDistanceFine;

			// only set for ECAL
            const float maxDistanceToTrack = (granularity >= pandora::COARSE) ? m_maxDistanceToTrackCoarse : m_maxDistanceToTrackFine;

            const pandora::CartesianVector caloHitsVector(pTestCaloHit->GetPositionVector() - pCaloHit->GetPositionVector());
            const float caloHitsAngle = caloHitsVector.GetOpeningAngle(extrapolatedMomentum);
            const float longitudinalDistance = caloHitsVector.GetMagnitude()*cos(caloHitsAngle);
            const float transverseDistance = caloHitsVector.GetMagnitude()*sin(caloHitsAngle);

            pandora::CartesianVector distanceToHelix(0.f, 0.f, 0.f);
            helix.GetDistanceToPoint(pTestCaloHit->GetPositionVector(), distanceToHelix);

			//std::cout << "firstLayer: " << firstLayer << ", pTestCaloHit layer: " << pTestCaloHit->GetPseudoLayer() << std::endl;

			// FIXME
			//if(caloHitsAngle > 0.5) continue;

			// FIXME
			// this never happens since the range of angle is [0, pi]
            if(longitudinalDistance < 0.f)
              continue;

            if(transverseDistance > maxTransverseDistance)
			{
			    // check transverse distance between two hits for layers after the fifth one
				// check last connector (or last average connector): angle with next connetor and get local transverse distance
#if 0
				if(pTestCaloHit->GetPseudoLayer() - firstLayer > 5)
				{
					const float maxLocalTransversDistance = 30.;

					const pandora::CartesianVector fromHitDirection = GetFromHitDirection(pCaloHit);
					const float localCaloHitsAngle = caloHitsVector.GetOpeningAngle(fromHitDirection);
                    const float localTransversDistance = caloHitsVector.GetMagnitude()*sin(localCaloHitsAngle);

				    if (localTransversDistance > maxLocalTransversDistance)
				    {
						continue;
				    }
				}
				else
				{ 
					continue;
				}
#endif

				continue;
			}

			// TODO
			// only check for first 5 layers after starting layer (ECAL)
            if(distanceToHelix.GetZ() > maxDistanceToTrack)
              continue;

            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Connect(pCaloHit, pTestCaloHit, arbor_content::FORWARD_DIRECTION));

            // add it if not done, this will update the starting hit vector
            if(std::find(caloHitVector.begin(), caloHitVector.end(), pTestCaloHit) == caloHitVector.end())
              caloHitVector.push_back(pTestCaloHit);
          }
        }

        ++plIter;
      }
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode TrackDrivenSeedingTool::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_maxInitialPseudoLayer = 3;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxInitialPseudoLayer", m_maxInitialPseudoLayer));

    m_maxInitialTrackDistance = 20.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxInitialTrackDistance", m_maxInitialTrackDistance));

	// unused
    m_maxInitialTrackAngle = 0.9f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxInitialTrackAngle", m_maxInitialTrackAngle));

    m_maxConnectionPseudoLayer = 4;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxConnectionPseudoLayer", m_maxConnectionPseudoLayer));

    m_maxTransverseDistanceFine = 20.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTransverseDistanceFine", m_maxTransverseDistanceFine));

    m_maxTransverseDistanceCoarse = 40.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTransverseDistanceCoarse", m_maxTransverseDistanceCoarse));

    m_maxDistanceToTrackFine = std::numeric_limits<float>::max(); // it means this is unused if not set in steering file
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxDistanceToTrackFine", m_maxDistanceToTrackFine));

    m_maxDistanceToTrackCoarse = std::numeric_limits<float>::max(); // it means this is unused if not set in steering file
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxDistanceToTrackCoarse", m_maxDistanceToTrackCoarse));

    m_shouldUseIsolatedHits = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldUseIsolatedHits", m_shouldUseIsolatedHits));

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

