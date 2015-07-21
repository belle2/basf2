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

  /** calculates the angle between the hits/vectors (RZ),
   * returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleRZFull instead).
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class AngleRZSimple : public SelectionVariable< PointType , float > {
  public:

    /** calculates the angle between the hits/vectors (RZ), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleRZFull instead)  */
    static float value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, float> Helper;

      B2Vector3<float> outerVector = Helper::doAMinusB(outerHit, centerHit);
      B2Vector3<float> innerVector = Helper::doAMinusB(centerHit, innerHit);

      B2Vector3<float> rzVecAB(outerVector.Perp(), outerVector[2], 0.);
      B2Vector3<float> rzVecBC(innerVector.Perp(), innerVector[2], 0.);

      return Helper::calcAngle2D(rzVecAB, rzVecBC);
    } // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleRZFull instead)
  };

}
