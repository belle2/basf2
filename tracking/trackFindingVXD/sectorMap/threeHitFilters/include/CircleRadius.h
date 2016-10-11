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
#include <tracking/trackFindingVXD/filterTools/SelectionVariableHelper.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/CircleCenterXY.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

namespace Belle2 {

  /** calculates the estimation of the circle radius of the 3-hit-tracklet, returning unit: cm.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class CircleRadius : public SelectionVariable< PointType , double > {
  public:
    /** return name of the Class */
    static const std::string name(void) {return "CircleRadius"; };


    /** calculates the estimation of the circle radius of the 3-hit-tracklet, returning unit: cm. */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D circleCenter = CircleCenterXY<PointType>::value(outerHit, centerHit, innerHit);

      return Helper::calcRadius(outerHit, centerHit, innerHit, circleCenter);
    } // return unit: cm
  };

}
