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
#include <tracking/trackFindingVXD/filterTools/SelectionVariableHelper.h>
#include <framework/geometry/B2Vector3.h>

#define DELTAPT_NAME DeltaPt

namespace Belle2 {

  /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c.
   * */
  template <typename PointType >
  class DELTAPT_NAME : public SelectionVariable< PointType, 4, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DELTAPT_NAME);


    /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c */
    static double value(const PointType& outerHit, const PointType& outerCenterHit, const PointType& innerCenterHit,
                        const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D outerCircleCenter =
        CircleCenterXY<PointType>::value(outerHit, outerCenterHit, innerCenterHit);
      double outerCircleRadius =
        CircleRadius<PointType>::calcAvgDistanceXY(outerHit, outerCenterHit, innerCenterHit, outerCircleCenter);

      B2Vector3D innerCircleCenter =
        CircleCenterXY<PointType>::value(outerCenterHit, innerCenterHit, innerHit);
      double innerCircleRadius =
        CircleRadius<PointType>::calcAvgDistanceXY(outerCenterHit, innerCenterHit, innerHit, innerCircleCenter);

      return fabs(Helper::calcPt(outerCircleRadius - innerCircleRadius));
    } // return unit: GeV/c
  };

}
