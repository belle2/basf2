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

#include <tracking/trackFindingVXD/FilterTools/SelectionVariable.h>
#include <tracking/trackFindingVXD/FilterTools/SelectionVariableHelper.h>
#include <tracking/vectorTools/B2Vector3.h>
#include <math.h>

namespace Belle2 {

  /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c.
   * */
  template <typename PointType >
  class DeltaPt : public SelectionVariable< PointType , float > {
  public:

    /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c */
    static float value(const PointType& outerHit, const PointType& outerCenterHit, const PointType& innerCenterHit,
                       const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, float> Helper;

      B2Vector3<float> outerCircleCenter = Helper::calcCircleCenter(outerHit, outerCenterHit, innerCenterHit);
      float outerCircleRadius = Helper::calcRadius(outerHit, outerCenterHit, innerCenterHit, outerCircleCenter);

      B2Vector3<float> innerCircleCenter = Helper::calcCircleCenter(outerCenterHit, innerCenterHit, innerHit);
      float innerCircleRadius = Helper::calcRadius(outerCenterHit, innerCenterHit, innerHit, innerCircleCenter);

      return fabs(Helper::calcPt(outerCircleRadius - innerCircleRadius));
    } // return unit: GeV/c
  };

}
