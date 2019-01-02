/// \file CaloHitNeighborSearchHelper.cc
/*
 *
 * CaloHitNeighborSearchHelper.cc source template automatically generated by a class generator
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
 &* 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#include "Api/PandoraContentApi.h"
#include "Objects/OrderedCaloHitList.h"
#include "ArborHelpers/CaloHitNeighborSearchHelper.h"
#include "ArborApi/ArborContentApi.h"
#include "ArborObjects/CaloHit.h"


using namespace mlpack;
using namespace mlpack::math;

namespace arbor_content
{
	arma::mat CaloHitNeighborSearchHelper::m_caloHitsMatrix(3,1);
	const pandora::CaloHitList* CaloHitNeighborSearchHelper::m_pCaloHitList(nullptr);
	pandora::CaloHitVector CaloHitNeighborSearchHelper::m_caloHitVector;
	mlpack::neighbor::KNN CaloHitNeighborSearchHelper::m_neighborSearch(m_caloHitsMatrix);

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitNeighborSearchHelper::FillMatixFromCaloHits(const pandora::CaloHitVector& caloHitVector, arma::mat& caloHitsMatrix)
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

  pandora::StatusCode CaloHitNeighborSearchHelper::BuildNeighborSearch(const pandora::CaloHitList *const pCaloHitList)
  {
	  if(pCaloHitList != m_pCaloHitList)
	  {
		  std::cout << "build new matrix for NeighborSearch..." << std::endl;
		  m_caloHitVector.clear();
	      m_caloHitVector.insert(m_caloHitVector.begin(), pCaloHitList->begin(), pCaloHitList->end());
		  FillMatixFromCaloHits(m_caloHitVector, m_caloHitsMatrix);

	      // the relatively time-comsuming part
	      m_neighborSearch.Train(m_caloHitsMatrix);
	  
		  m_pCaloHitList = pCaloHitList;
	  }

	  return pandora::STATUS_CODE_SUCCESS;
  }

  pandora::StatusCode CaloHitNeighborSearchHelper::SearchNeighbourHits(pandora::CartesianVector testPosition, int nNeighbor, 
		  pandora::CaloHitList& neighborHits)
  {
	  neighborHits.clear();

	  /////
      arma::mat testPoint(3, 1);

	  testPoint.col(0)[0] = testPosition.GetX();
	  testPoint.col(0)[1] = testPosition.GetY();
	  testPoint.col(0)[2] = testPosition.GetZ();

	  //std::cout << "point: " << testPoint.col(0)[0] << ", " << testPoint.col(0)[1] << ", " << testPoint.col(0)[2] << std::endl;

	  arma::Mat<size_t> resultingNeighbors;
	  arma::mat resultingDistances;

      m_neighborSearch.Search(testPoint, nNeighbor, resultingNeighbors, resultingDistances);

      for(size_t j=0; j < resultingNeighbors.n_elem; ++j)
      {
      	size_t neighbor = resultingNeighbors[j];
      	double hitsDist = resultingDistances[j];

#if 0
   	    auto posVec = m_caloHitVector.at(neighbor)->GetPositionVector();
   
      	std::cout <<  "    -> neighbor " << neighbor << ", hits distance: " << hitsDist 
   	  	 << ", X = " << posVec.GetX() << ", " << posVec.GetY() << ", " << posVec.GetZ() << std::endl;
#endif

		neighborHits.push_back( m_caloHitVector.at(neighbor) );
      }

   	  return pandora::STATUS_CODE_SUCCESS;
  }

}