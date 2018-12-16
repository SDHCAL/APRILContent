/// \file CaloHitRangeSearchHelper.cc
/*
 *
 * CaloHitRangeSearchHelper.cc source template automatically generated by a class generator
 * Creation date : lun. mars 30 2015
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


#include "ArborHelpers/CaloHitRangeSearchHelper.h"
#include "Api/PandoraContentApi.h"
#include "ArborApi/ArborContentApi.h"
#include "ArborObjects/CaloHit.h"


using namespace mlpack;
using namespace mlpack::range;
using namespace mlpack::math;

namespace arbor_content
{
	const pandora::CaloHitList* CaloHitRangeSearchHelper::m_pCaloHitList = nullptr;
	pandora::CaloHitVector CaloHitRangeSearchHelper::m_caloHitVector;
	arma::mat CaloHitRangeSearchHelper::m_caloHitsMatrix = arma::mat(3,1);

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::FillMatixFromCaloHits(const pandora::CaloHitVector& caloHitVector, arma::mat& caloHitsMatrix)
  {
	  // first hit 
      arma::mat matrix(3, 1);

	  pandora::CartesianVector caloHitPosition0 = caloHitVector.at(0)->GetPositionVector();
	  matrix.col(0) = arma::vec( { caloHitPosition0.GetX(), caloHitPosition0.GetY(), caloHitPosition0.GetZ() } );

	  // other hits
	  //std::cout << "caloHitVector.size : " << caloHitVector.size() << std::endl;
	  matrix.insert_cols(1, caloHitVector.size() - 1); 

	  for(int i = 1; i < caloHitVector.size(); ++i)
	  {
	      pandora::CartesianVector caloHitPosition = caloHitVector.at(i)->GetPositionVector();
	      matrix.col(i) = arma::vec( { caloHitPosition.GetX(), caloHitPosition.GetY(), caloHitPosition.GetZ() } );
	  }

	  caloHitsMatrix = matrix;
	  
	  return pandora::STATUS_CODE_SUCCESS;
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::GetNeighbourHitsInRange(const pandora::CaloHitList *const pCaloHitList, 
		  const pandora::CaloHit *const pCaloHit, float distance, pandora::CaloHitVector& hitsInRange)
  {
	  hitsInRange.clear();

	  if(pCaloHitList != m_pCaloHitList)
	  {
		  m_caloHitVector.clear();
	      m_caloHitVector.insert(m_caloHitVector.begin(), pCaloHitList->begin(), pCaloHitList->end());
		  FillMatixFromCaloHits(m_caloHitVector, m_caloHitsMatrix);

		  m_pCaloHitList = pCaloHitList;
	  }

	  RangeSearch<> rangeSearch(m_caloHitsMatrix);

	  Range range(0., distance);

	  /////
      arma::mat testPoint(3, 1);

	  pandora::CartesianVector caloHitPosVec = pCaloHit->GetPositionVector();
	  testPoint.col(0)[0] = caloHitPosVec.GetX();
	  testPoint.col(0)[1] = caloHitPosVec.GetY();
	  testPoint.col(0)[2] = caloHitPosVec.GetZ();

	  //std::cout << "point: " << testPoint.col(0)[0] << ", " << testPoint.col(0)[1] << ", " << testPoint.col(0)[2] << std::endl;

      std::vector<std::vector<size_t> > resultingNeighbors;
      std::vector<std::vector<double> > resultingDistances;
      rangeSearch.Search(testPoint, range, resultingNeighbors, resultingDistances);

	  //std::cout << "resultingNeighbors size: " <<  resultingNeighbors.size() << ", resultingDistances size: " << resultingDistances.size() << std::endl;

      if(resultingNeighbors.size() != 1 ) 
	  {
		  std::cout << "error " << std::endl;

   	      return pandora::STATUS_CODE_SUCCESS;
	  }

      std::vector<size_t>& neighbors = resultingNeighbors.at(0);
      std::vector<double>& distances = resultingDistances.at(0);
   
      for(size_t j=0; j < neighbors.size(); ++j)
      {
      	size_t neighbor = neighbors.at(j);
      	double hitsDist = distances.at(j);

#if 0
   	    auto posVec = m_caloHitVector.at(neighbor)->GetPositionVector();
   
      	std::cout <<  "    -> neighbor " << neighbor << ", hits distance: " << hitsDist 
   	  	 << ", X = " << posVec.GetX() << ", " << posVec.GetY() << ", " << posVec.GetZ() << std::endl;
#endif

		hitsInRange.push_back( m_caloHitVector.at(neighbor) );
      }

   	  return pandora::STATUS_CODE_SUCCESS;
  }
} 

