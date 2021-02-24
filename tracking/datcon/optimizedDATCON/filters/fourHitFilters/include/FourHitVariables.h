/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Wessel                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/datcon/optimizedDATCON/filters/threeHitFilters/ThreeHitVariables.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.>

#include <cmath>

namespace Belle2 {

  /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: cm */
  /** TODO: a straight line exception is thrown and not catched (at least here) in calcCircleCenter */
  double getCircleRadiusDifference(const B2Vector3D& outerHit, const B2Vector3D& outerCenterHit,
                                   const B2Vector3D& innerCenterHit, const B2Vector3D& innerHit)
  {
    B2Vector3D outerCircleCenter = getCircleCenterXY(outerHit, outerCenterHit, innerCenterHit);
    if (std::isnan(outerCircleCenter)) return NAN;
    double outerCircleRadius = calcAvgDistanceXY(outerHit, outerCenterHit, innerCenterHit, outerCircleCenter);

    B2Vector3D innerCircleCenter = getCircleCenterXY(outerCenterHit, innerCenterHit, innerHit);
    if (std::isnan(innerCircleCenter)) return NAN;
    double innerCircleRadius = calcAvgDistanceXY(outerCenterHit, innerCenterHit, innerHit, innerCircleCenter);

    return outerCircleRadius - innerCircleRadius;
  } // return unit: cm


  /** calculates the distance between the estimated circle centers (using 2 subsets of given hits) in the xy-plane, returning unit: cm */
  double getCircleCenterPositionDifference(const B2Vector3D& outerHit, const B2Vector3D& outerCenterHit,
                                           const B2Vector3D& innerCenterHit, const B2Vector3D& innerHit)
  {
    B2Vector3D outerCircleCenter = getCircleCenterXY(outerHit, outerCenterHit, innerCenterHit);
    if (std::isnan(outerCircleCenter)) return NAN;
    B2Vector3D innerCircleCenter = getCircleCenterXY(outerCenterHit, innerCenterHit, innerHit);
    if (std::isnan(innerCircleCenter)) return NAN;

    return fabs(outerCircleCenter.Perp() - innerCircleCenter.Perp());
  } // return unit: GeV/c


  /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c */
  double getDeltaPT(const B2Vector3D& outerHit, const B2Vector3D& outerCenterHit,
                    const B2Vector3D& innerCenterHit, const B2Vector3D& innerHit)
  {
    B2Vector3D outerCircleCenter = getCircleCenterXY(outerHit, outerCenterHit, innerCenterHit);
    if (std::isnan(outerCircleCenter)) return NAN;
    double outerCircleRadius = calcAvgDistanceXY(outerHit, outerCenterHit, innerCenterHit, outerCircleCenter);

    B2Vector3D innerCircleCenter = getCircleCenterXY(outerCenterHit, innerCenterHit, innerHit);
    if (std::isnan(innerCircleCenter)) return NAN;
    double innerCircleRadius = calcAvgDistanceXY(outerCenterHit, innerCenterHit, innerHit, innerCircleCenter);

    // 0.3 * B * R, but with R in cm instead of m -> (0.3 -> 0.003)
    // 0.00299792458 * 1.5 = 0.00449688687
    return fabs(0.00449688687 * (outerCircleRadius - innerCircleRadius));
  } // return unit: GeV/c

}
