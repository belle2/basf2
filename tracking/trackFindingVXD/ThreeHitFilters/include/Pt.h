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

  /** calculates the estimation of the transverse momentum of the 3-hit-tracklet, returning unit: GeV/c.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class Pt : public SelectionVariable< PointType , float > {
  public:

    /** calculates the estimation of the transverse momentum of the 3-hit-tracklet, returning unit: GeV/c */
    static float value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, float> Helper;

      B2Vector3<float> circleCenter = Helper::calcCircleCenter(outerHit, centerHit, innerHit);
      float circleRadius = Helper::calcRadius(outerHit, centerHit, innerHit, circleCenter);

      return circleRadius * Helper::s_MagneticFieldFactor;
    } // return unit: GeV/c
  };

}
