/*
 *
 * GlobalConnectorCleaningAlgorithm.h header template automatically generated by a class generator
 * Creation date : mar. avr. 7 2015
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


#ifndef GLOBALCONNECTORCLEANINGALGORITHM_H
#define GLOBALCONNECTORCLEANINGALGORITHM_H

#include "Pandora/Algorithm.h"
#include "Pandora/PandoraInternal.h"

namespace pandora { class OrderedCaloHitList; }

namespace arbor_content
{

class CaloHit;
class Connector;
class ReferenceVectorToolBase;
class OrderParameterToolBase;

/** 
 * @brief GlobalConnectorCleaningAlgorithm class
 */ 
class GlobalConnectorCleaningAlgorithm : public pandora::Algorithm
{
public:
	/**
	 *  @brief  Factory class for instantiating algorithm
	 */
	class Factory : public pandora::AlgorithmFactory
	{
	public:
		pandora::Algorithm *CreateAlgorithm() const;
	};

private:
	/**
	 *
	 */
	class CleaningHelper
	{
	public:
		const CaloHit          *m_pCaloHit;
		pandora::CaloHitList     m_deleteConnectionCaloHitList;
	};

	typedef std::vector<CleaningHelper> CleaningHelperList;

	/**
	 *
	 */
	pandora::StatusCode Clean(const pandora::CaloHitList *const pCaloHitList) const;

	/**
	 *
	 */
	pandora::StatusCode Clean(const pandora::OrderedCaloHitList &orderedcaloHitList) const;

	pandora::StatusCode Run();
	pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

	unsigned int                      m_cleaningStrategy;
	float                              m_orderParameterCut;

	ReferenceVectorToolBase            *m_pReferenceVectorTool;
	OrderParameterToolBase             *m_pOrderParameterTool;
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *GlobalConnectorCleaningAlgorithm::Factory::CreateAlgorithm() const
{
    return new GlobalConnectorCleaningAlgorithm();
}

} 

#endif  //  GLOBALCONNECTORCLEANINGALGORITHM_H