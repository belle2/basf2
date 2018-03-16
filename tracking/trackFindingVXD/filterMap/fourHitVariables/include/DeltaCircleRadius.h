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

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/CircleCenterXY.h>
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/CircleRadius.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

#define DELTACIRCLERADIUS_NAME DeltaCircleRadius

namespace Belle2 {

  /** calculates delta-circleRadius-value (difference in circle radii of 2 subsets of the hits), returning unit: cm.
   * */
  template <typename PointType >
  class DELTACIRCLERADIUS_NAME : public SelectionVariable< PointType , 4, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DELTACIRCLERADIUS_NAME);



    /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: cm */
    /** TODO: a straight line exception is thrown and not catched (at least here) in calcCircleCenter */
    static double value(const PointType& outerHit, const PointType& outerCenterHit, const PointType& innerCenterHit,
                        const PointType& innerHit)
    {

      B2Vector3D outerCircleCenter = CircleCenterXY<PointType>::value(outerHit, outerCenterHit, innerCenterHit);
      double outerCircleRadius =
        CircleRadius<PointType>::calcAvgDistanceXY(outerHit, outerCenterHit, innerCenterHit, outerCircleCenter);

      B2Vector3D innerCircleCenter = CircleCenterXY<PointType>::value(outerCenterHit, innerCenterHit, innerHit);
      double innerCircleRadius =
        CircleRadius<PointType>::calcAvgDistanceXY(outerCenterHit, innerCenterHit, innerHit, innerCircleCenter);

      return outerCircleRadius - innerCircleRadius;
    } // return unit: cm
  };

}
