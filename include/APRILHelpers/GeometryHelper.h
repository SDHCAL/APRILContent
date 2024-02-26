/// \file GeometryHelper.h
/*
 *
 * GeometryHelper.h header template automatically generated by a class generator
 * Creation date : jeu. avr. 9 2015
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


#ifndef GEOMETRYHELPER_H
#define GEOMETRYHELPER_H

#include "Pandora/PandoraInternal.h"
#include "Pandora/Pandora.h"
#include "Pandora/StatusCodes.h"
#include "Objects/CartesianVector.h"
#include "Pandora/PandoraEnumeratedTypes.h"

namespace april_content
{

/** 
 *  @brief  GeometryHelper class
 */ 
class GeometryHelper 
{
public:
	 /**
	  *  @brief  Get the closest distance between the two lines
	  *
	  *  @param  point1 on the line 1
	  *  @param  direction1 of the line1
	  *  @param  point2 on the line 2
	  *  @param  direction2 of the line 2
	  *  @param  the closest distance to receive
	  */
	 static pandora::StatusCode GetClosestDistanceBetweenLines(const pandora::CartesianVector &point1, const pandora::CartesianVector &direction1,
	 		const pandora::CartesianVector &point2, const pandora::CartesianVector &direction2, float &distance);

	 /**
	  *  @brief  Get the closest distance between a point and a line
	  *
	  *  @param  point1 on the line
	  *  @param  direction1 of the line
	  *  @param  point2 in the space
	  *  @param  the closest distance between the point and the line to receive
	  */
	 static pandora::StatusCode GetClosestDistanceToLine(const pandora::CartesianVector &point1, const pandora::CartesianVector &direction1,
	 		const pandora::CartesianVector &point2, float &impactParameter);

	 /**
	  *  @brief  Get the projection of a point on the line.
	  *
	  *  @param  linePoint on the line
	  *  @param  direction1 of the line
	  *  @param  point in the space
	  *  @param  projection the projection vector on the line to receive
	  */
	 static pandora::StatusCode GetProjectionOnLine(const pandora::CartesianVector &linePoint, const pandora::CartesianVector &direction,
	 		const pandora::CartesianVector &point, pandora::CartesianVector &projection);

	 /**
	  *  @brief  Get the two crossing points of two lines at the closest distance approach
	  *
	  *  @param  point1 on the line 1
	  *  @param  direction1 of the line 1
	  *  @param  point2 on the line 2
	  *  @param  direction2 of the line 2
	  *  @param  crossingPoint1 the crossing point on the line 1 to receive
	  *  @param  crossingPoint2 the crossing point on the line 2 to receive
	  */
	 static pandora::StatusCode GetCrossingPointsBetweenLines(const pandora::CartesianVector &point1, const pandora::CartesianVector &direction1,
	 		const pandora::CartesianVector &point2, const pandora::CartesianVector &direction2,
	 		pandora::CartesianVector &crossingPoint1, pandora::CartesianVector &crossingPoint2);

	 /**
	  *  @brief  Get the crossing point between a line and a plane in space. The equation plane is defined as :
	  *
	  *    P : a*x + b*y + c*z + d = 0
	  *
	  *  @param  linePoint a point on the line
	  *  @param  lineDirection the line direction vector
	  *  @param  planeA the a component in the plane equation
	  *  @param  planeB the b component in the plane equation
	  *  @param  planeC the c component in the plane equation
	  *  @param  planeD the d component in the plane equation
	  *  @param  crossingPoint the crossing point vector position to receive
	  */
	 static pandora::StatusCode GetLinePlaneCrossingPoint(const pandora::CartesianVector &linePoint, const pandora::CartesianVector &lineDirection,
			 float planeA, float planeB, float planeC, float planeD, pandora::CartesianVector &crossingPoint);

	 /**
	  *  @brief  Get the gap size between the ECal end cap and the HCal end cap
	  *
	  *  @param  pandora the pandora instance to access geometry parameters
	  *  @param  gapSize the gap size to receive
	  */
	 static pandora::StatusCode GetECalHCalEndCapGapSize(const pandora::Pandora &pandora, float &gapSize);

	 /**
	  *  @brief  Get the gap size between the ECal barrel and the HCal barrel
	  *
	  *  @param  pandora the pandora instance to access geometry parameters
	  *  @param  gapSize the gap size to receive
	  */
	 static pandora::StatusCode GetECalHCalBarrelGapSize(const pandora::Pandora &pandora, float &gapSize);

	 /**
	  *  @brief  Get the gap size between the tracker and ECal end cap
	  *
	  *  @param  pandora the pandora instance to access geometry parameters
	  *  @param  gapSize the gap size to receive
	  */
	 static pandora::StatusCode GetTrackerECalEndCapGapSize(const pandora::Pandora &pandora, float &gapSize);

	 /**
	  *  @brief  Get the gap size between the tracker and ECal barrel
	  *
	  *  @param  pandora the pandora instance to access geometry parameters
	  *  @param  gapSize the gap size to receive
	  */
	 static pandora::StatusCode GetTrackerECalBarrelGapSize(const pandora::Pandora &pandora, float &gapSize);

	 /**
	  *  @brief  Get the gap separation in the end cap region between two calo hits
	  *
	  *  @param  pandora the pandora instance to access geometry parameters
	  *  @param  pEcalCaloHit the ecal calo hit address in the end cap region
	  *  @param  pHcalCaloHit the hcal calo hit address in the end cap region
	  *  @param  gapSeparation the gap separation distance to receive
	  */
	 static pandora::StatusCode GetEndcapGapSeparation(const pandora::Pandora &pandora, const pandora::CaloHit *const pEcalCaloHit,
			 const pandora::CaloHit *const pHcalCaloHit, float &gapSeparation);

	 /**
	  *  @brief  Get the gap separation in the barrel region between two calo hits
	  *
	  *  @param  pandora the pandora instance to access geometry parameters
	  *  @param  pEcalCaloHit the ecal calo hit address in the barrel region
	  *  @param  pHcalCaloHit the hcal calo hit address in the barrel region
	  *  @param  gapSeparation the gap separation distance to receive
	  */
	 static pandora::StatusCode GetBarrelGapSeparation(const pandora::Pandora &pandora, const pandora::CaloHit *const pEcalCaloHit,
			 const pandora::CaloHit *const pHcalCaloHit, float &gapSeparation);

	 /**
	  *  @brief  Get the normale vector at outer detector face in the phi region where the point is contained
	  *
	  *  @param  pandora the pandora instance to access geometry parameters
	  *  @param  type the sub detector type (requires access to outer symetry order)
	  *  @param  point a point in space that fixes the phi region
	  *  @param  normaleVector the normale vector at the outer detector face to receive
	  */
	 static pandora::StatusCode GetOuterNormaleVector(const pandora::Pandora &pandora, pandora::SubDetectorType type, const pandora::CartesianVector &point,
			 pandora::CartesianVector &normaleVector);

	 /**
	  *  @brief  Get the normale vector at inner detector face in the phi region where the point is contained
	  *
	  *  @param  pandora the pandora instance to access geometry parameters
	  *  @param  type the sub detector type (requires access to inner symmetry order)
	  *  @param  point a point in space that fixes the phi region
	  *  @param  normaleVector the normal vector at the inner detector face to receive
	  */
	 static pandora::StatusCode GetInnerNormaleVector(const pandora::Pandora &pandora, pandora::SubDetectorType type, const pandora::CartesianVector &point,
			 pandora::CartesianVector &normaleVector);

	 /**
	  *  @brief  Get the projection of a point on the helix
	  *
	  *  @param  helix the helix to consider
	  *  @param  point the point in space to project on the helix
	  *  @param  projection the projection point to receive
	  */
	 static pandora::StatusCode GetProjectionOnHelix(const pandora::Helix &helix, const pandora::CartesianVector &point, pandora::CartesianVector &projection);

	 /**
	  *  @brief  Get the distance between a point and the helix
	  *
	  *  @param  helix the helix to consider
	  *  @param  point a point in space
	  *  @param  distanceToHelix the distance to the helix to receive
	  */
	 static pandora::StatusCode GetDistanceToHelix(const pandora::Helix &helix, const pandora::CartesianVector &point, float &distanceToHelix);
}; 

} 

#endif  //  GEOMETRYHELPER_H
