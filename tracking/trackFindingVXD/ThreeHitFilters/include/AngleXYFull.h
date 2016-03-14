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
#include <framework/geometry/B2Vector3.h>
#include <math.h>

namespace Belle2 {

  /** calculates the angle between the hits/vectors (XY), returning unit: angle in degrees.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class AngleXYFull : public SelectionVariable< PointType , double > {
  public:

    /** calculates the angle between the hits/vectors (XY), returning unit: angle in degrees */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D outerVector = Helper::doAMinusB(outerHit, centerHit);
      B2Vector3D innerVector = Helper::doAMinusB(centerHit, innerHit);

      double result = Helper::fullAngle2D(outerVector, innerVector); // 0-pi
      result *= double(180. / M_PI);
      return Helper::checkValid(result);
    } // return unit: ° (0 - 180°)
  };

}
