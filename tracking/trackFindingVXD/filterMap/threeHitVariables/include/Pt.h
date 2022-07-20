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

#define PT_NAME Pt

namespace Belle2 {

  /** calculates the estimation of the transverse momentum of the 3-hit-tracklet, returning unit: GeV/c.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class PT_NAME : public SelectionVariable< PointType, 3, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(PT_NAME);

    /** calculates the estimation of the transverse momentum of the 3-hit-tracklet, returning unit: GeV/c */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D circleCenter = CircleCenterXY<PointType>::value(outerHit, centerHit, innerHit);
      double circleRadius = CircleRadius<PointType>::calcAvgDistanceXY(outerHit, centerHit, innerHit, circleCenter);

      return circleRadius * Helper::s_MagneticFieldFactor;
    } // return unit: GeV/c
  };

}
