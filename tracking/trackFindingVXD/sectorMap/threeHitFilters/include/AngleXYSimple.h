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
#include <framework/geometry/B2Vector3.h>
#include <math.h>

namespace Belle2 {

  /** calculates the angle between the hits/vectors (XY),
   * returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleXYFull instead):
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class AngleXYSimple : public SelectionVariable< PointType , double > {
  public:

    /** calculates the angle between the hits/vectors (XY),
     * returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleXYFull instead) */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D outerVector = Helper::doAMinusB(outerHit, centerHit);
      B2Vector3D innerVector = Helper::doAMinusB(centerHit, innerHit);

      double result = (outerVector.X() * innerVector.X() + outerVector.Y() * innerVector.Y()) / (outerVector.Perp2() *
                      innerVector.Perp2());

      return Helper::checkValid(result);
    } // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleXYFull instead)
  };

}
