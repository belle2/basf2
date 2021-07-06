/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/CircleCenterXY.h>
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/CircleRadius.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/CosDirectionXY.h>
#include <framework/geometry/B2Vector3.h>
#include <cmath>

#define DELTASLOPEZOVERS_NAME DeltaSlopeZoverS

namespace Belle2 {

  /** compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class DELTASLOPEZOVERS_NAME : public SelectionVariable< PointType , 3, double > {
  public:

    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DELTASLOPEZOVERS_NAME);

    /** compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane. */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {

      B2Vector3D cCenter = CircleCenterXY<PointType>::value(outerHit, centerHit, innerHit);
      double circleRadius = CircleRadius<PointType>::calcAvgDistanceXY(outerHit, centerHit, innerHit, cCenter);
      B2Vector3D vecOuter2cC((outerHit.X() - cCenter.X()),
                             (outerHit.Y() - cCenter.Y()),
                             (outerHit.Z() - cCenter.Z()));
      B2Vector3D vecCenter2cC((centerHit.X() - cCenter.X()),
                              (centerHit.Y() - cCenter.Y()),
                              (centerHit.Z() - cCenter.Z()));
      B2Vector3D vecInner2cC((innerHit.X() - cCenter.X()),
                             (innerHit.Y() - cCenter.Y()),
                             (innerHit.Z() - cCenter.Z()));

      // WARNING: this is only approximately S (valid in the limit of small angles) but might be OK for this use!!!
      //  want to replace id with 2*sin ( alfa ) * circleRadius
      double alfaOCr = acos(CosDirectionXY<B2Vector3D>::value(vecOuter2cC, vecCenter2cC)) * circleRadius ;
      double alfaCIr = acos(CosDirectionXY<B2Vector3D>::value(vecCenter2cC, vecInner2cC)) * circleRadius ;

      // Beware of z>r!:
      double result = (asin(double(outerHit.Z() - centerHit.Z()) / alfaOCr)) - asin(double(centerHit.Z() - innerHit.Z()) / alfaCIr);

      return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
    }
  };

}
