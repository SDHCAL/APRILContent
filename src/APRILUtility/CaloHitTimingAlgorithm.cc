/// \file CaloHitTimingAlgorithm.cc
/*
 *
 * CaloHitTimingAlgorithm.cc source template automatically generated by a class generator
 * Creation date : jeu. juin 2 2016
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


#include "APRILUtility/CaloHitTimingAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"
#include "APRILApi/APRILContentApi.h"
#include "APRILObjects/CaloHit.h"
#include "APRILHelpers/GeometryHelper.h"


namespace april_content
{

  pandora::StatusCode CaloHitTimingAlgorithm::Run()
  {


    const pandora::CaloHitList *pCaloHitList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

	std::cout << "  ---> CaloHit list size: " << pCaloHitList->size() << std::endl;

#if 0
    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), endIter = pCaloHitList->end() ;
        endIter != iter ; ++iter)
    {
        const april_content::CaloHit *const pCaloHit(dynamic_cast<const april_content::CaloHit *>(*iter));
		const pandora::CartesianVector position(pCaloHit->GetPositionVector());
		std::cout << "hit position: " << " " << position.GetX() << " " << position.GetY() << " " << position.GetZ() 
			      << " time: " << pCaloHit->GetTime() << std::endl;
	}
#endif

	if(m_timing) 
	{
		// a calo hit list for early hits
		pandora::CaloHitList caloHitList;

		// a calo hit list for late hits
		pandora::CaloHitList caloLateHitList;
	

		for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(); iter != pCaloHitList->end(); ++iter)
		{
			float hitTime = (*iter)->GetTime();
			const pandora::CartesianVector& hitPos  = (*iter)->GetPositionVector();
#if 0
			std::cout << "hit time: " << hitTime << std::endl;
#endif

			caloHitsMonitor->Fill(hitPos.GetX(), hitPos.GetY(), hitPos.GetZ(), hitTime);

			if(hitTime < m_timeCut)
				caloHitList.push_back(*iter);
			else
				caloLateHitList.push_back(*iter);
		}

		// Save the current list in a list with the specified new name
		//std::cout << "calo hit list: " << caloHitList.size() << ", late hit list: " << caloLateHitList.size() << std::endl;

		//std::string caloHitName0("hahaList");
		//PandoraContentApi::GetCurrentListName<pandora::CaloHit>(*this, caloHitName0);
		//PandoraContentApi::DropCurrentList<pandora::CaloHit>(*this);
		//std::cout << "name: " << caloHitName0 << std::endl;

		std::string caloHitName("HitsAfterTiming");
		PandoraContentApi::SaveList<pandora::CaloHitList>(*this, caloHitList, caloHitName);
		PandoraContentApi::ReplaceCurrentList<pandora::CaloHit>(*this, caloHitName);

		std::string caloLateHitName("LateHits");
		PandoraContentApi::SaveList<pandora::CaloHitList>(*this, caloLateHitList, caloLateHitName);
    
		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList)); 
		std::cout << "  ---> CaloHit list size after timing: " << pCaloHitList->size() << std::endl;
	}

    return pandora::STATUS_CODE_SUCCESS;
  }


  pandora::StatusCode CaloHitTimingAlgorithm::Initialize()
  {
      std::cout << "timing:  "   << m_timing  << std::endl;
      std::cout << "time cut:  " << m_timeCut << std::endl;
      
	  //std::cout << "********* CaloHitTimingAlgorithm init ********" << std::endl;
	  caloHitsMonitor = new TNtupleD("calohits", "calohits", "x:y:z:t");
    
	  return pandora::STATUS_CODE_SUCCESS;
  }

/*
  CaloHitTimingAlgorithm::~CaloHitTimingAlgorithm()
  {
	  //std::cout << "********* CaloHitTimingAlgorithm destructor ********" << std::endl;
	  //if(caloHitsMonitor != NULL) delete caloHitsMonitor;

	  pandora::Algorithm::~Algorithm();
  }
*/

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitTimingAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
	m_timing = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ApplyTiming", m_timing));

    m_timeCut = 100.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "TimeCut", m_timeCut));

    return pandora::STATUS_CODE_SUCCESS;
  }

} 
