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

  /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c.
   * */
  template <typename PointType >
  class DeltaPt : public SelectionVariable< PointType , double > {
  public:
    /** return name of the Class */
    static const std::string name(void) {return "DeltaPt"; };

    /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c */
    static double value(const PointType& outerHit, const PointType& outerCenterHit, const PointType& innerCenterHit,
                        const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3<double> outerCircleCenter = CircleCenterXY<PointType>::value(outerHit, outerCenterHit, innerCenterHit);
      double outerCircleRadius = Helper::calcRadius(outerHit, outerCenterHit, innerCenterHit, outerCircleCenter);

      B2Vector3<double> innerCircleCenter = CircleCenterXY<PointType>::value(outerCenterHit, innerCenterHit, innerHit);
      double innerCircleRadius = Helper::calcRadius(outerCenterHit, innerCenterHit, innerHit, innerCircleCenter);

      return fabs(Helper::calcPt(outerCircleRadius - innerCircleRadius));
    } // return unit: GeV/c
  };

}
