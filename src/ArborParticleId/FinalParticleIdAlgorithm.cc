/// \file FinalParticleIdAlgorithm.cc
/*
 *
 * FinalParticleIdAlgorithm.cc source template automatically generated by a class generator
 * Creation date : mar. oct. 18 2016
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


#include "ArborParticleId/FinalParticleIdAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"

namespace arbor_content
{

  pandora::StatusCode FinalParticleIdAlgorithm::Run()
  {
    const pandora::PfoList *pPfoList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pPfoList));

	//std::cout << "FinalParticleIdAlgorithm..." << std::endl;

	//int nPFO(0);

    for (pandora::PfoList::const_iterator iter = pPfoList->begin(), iterEnd = pPfoList->end();
        iter != iterEnd; ++iter)
    {
      const pandora::ParticleFlowObject *const pPfo = *iter;

      const pandora::TrackList &trackList(pPfo->GetTrackList());
      const pandora::ClusterList &clusterList(pPfo->GetClusterList());
      const bool neutralPfo(trackList.empty());
      const int charge(pPfo->GetCharge());
#if 0
	  if(trackList.empty()) {
		  std::cout << "pfo " << ++nPFO << ", energy: " << pPfo->GetEnergy() << ", neutralPfo: " << neutralPfo << std::endl;
	  }
	  else {
		 std::cout << "pfo " << ++nPFO << ", energy: " << pPfo->GetEnergy() << ", neutralPfo: " << neutralPfo 
		           << ", track energy: " << std::endl;

		 for(pandora::TrackList::const_iterator trackIter = trackList.begin(); trackIter!=trackList.end(); ++trackIter) {
			 std::cout << "---> " << (*trackIter)->GetEnergyAtDca() << std::endl;
		 }
	  }
#endif

      // Consider only pfos with a single cluster and no track sibling relationships
      if ((clusterList.size() != 1) || this->ContainsSiblingTrack(trackList))
        continue;

      if(!neutralPfo && charge == 0)
        continue;

      // whether to force electron pid
      if( (std::abs(pPfo->GetParticleId()) == pandora::E_MINUS) && !m_forceElectronTagging )
        continue;

      // whether to force muon pid
      if( (std::abs(pPfo->GetParticleId()) == pandora::MU_MINUS) && !m_forceMuonTagging )
        continue;

      // whether to force photon pid
      if( (std::abs(pPfo->GetParticleId()) == pandora::PHOTON) && !m_forcePhotonTagging )
        continue;

      const pandora::Cluster *const pCluster(*clusterList.begin());
      const pandora::ParticleId *const pParticleId(PandoraContentApi::GetPlugins(*this)->GetParticleId());

      PandoraContentApi::ParticleFlowObject::Metadata metadata;

      if(pParticleId->IsElectron(pCluster))
      {
        metadata.m_particleId = (charge > 0) ? pandora::E_PLUS : pandora::E_MINUS;
      }
      else if(pParticleId->IsMuon(pCluster))
      {
        metadata.m_particleId = (charge > 0) ? pandora::MU_PLUS : pandora::MU_MINUS;
      }
      else if(charge == 0 && pParticleId->IsPhoton(pCluster))
      {
        metadata.m_particleId = pandora::PHOTON;
      }

      if(metadata.m_particleId.IsInitialized())
      {
        metadata.m_mass = pandora::PdgTable::GetParticleMass(metadata.m_particleId.Get());
        metadata.m_energy = std::sqrt(metadata.m_mass.Get() * metadata.m_mass.Get() + pPfo->GetMomentum().GetMagnitudeSquared());
		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::ParticleFlowObject::AlterMetadata(*this, pPfo, metadata));
      }
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  bool FinalParticleIdAlgorithm::ContainsSiblingTrack(const pandora::TrackList &trackList) const
  {
    for (pandora::TrackList::const_iterator iter = trackList.begin(), iterEnd = trackList.end(); iter != iterEnd; ++iter)
    {
      if (!(*iter)->GetSiblingList().empty())
      {
        return true;
      }
    }

    return false;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FinalParticleIdAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_forceMuonTagging = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ForceMuonTagging", m_forceMuonTagging));

    m_forceElectronTagging = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ForceElectronTagging", m_forceElectronTagging));

    m_forcePhotonTagging = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ForcePhotonTagging", m_forcePhotonTagging));

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

