/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/sectorMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/CircleCenterXY.h>
#include <tracking/trackFindingVXD/filterTools/SelectionVariableHelper.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

namespace Belle2 {

  /** compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class DeltaSlopeZoverS : public SelectionVariable< PointType , double > {
  public:
    /** return name of the Class */
    static const std::string name(void) {return "DeltaSlopeZoverS"; };

    /** compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane. */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D cCenter = CircleCenterXY<PointType>::value(outerHit, centerHit, innerHit);
      double circleRadius = Helper::calcRadius(outerHit, centerHit, innerHit, cCenter);
      B2Vector3D vecOuter2cC((outerHit.X() - cCenter.X()),
                             (outerHit.Y() - cCenter.Y()),
                             (outerHit.Z() - cCenter.Z()));
      B2Vector3D vecCenter2cC((centerHit.X() - cCenter.X()),
                              (centerHit.Y() - cCenter.Y()),
                              (centerHit.Z() - cCenter.Z()));
      B2Vector3D vecInner2cC((innerHit.X() - cCenter.X()),
                             (innerHit.Y() - cCenter.Y()),
                             (innerHit.Z() - cCenter.Z()));

      double alfaOCr = Helper::fullAngle2D(vecOuter2cC, vecCenter2cC) * circleRadius;
      double alfaCIr = Helper::fullAngle2D(vecCenter2cC, vecInner2cC) * circleRadius;

      // Beware of z>r!:
      double result = (asin(double(outerHit.Z() - centerHit.Z()) / alfaOCr)) - asin(double(centerHit.Z() - innerHit.Z()) / alfaCIr);

      return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
    }
  };

}
