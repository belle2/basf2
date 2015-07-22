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

  /** calculates the helixparameter describing the deviation in arc length per unit in z.
   * returning unit: radians*cm. */
  template <typename PointType >
  class DeltaSoverZ : public SelectionVariable< PointType , float > {
  public:

    /** calculates the helixparameter describing the deviation in arc length per unit in z.
     * returning unit: radians*cm */
    static float value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, float> Helper;

      B2Vector3<float> circleCenter = Helper::calcCircleCenter(outerHit, centerHit, innerHit);
      B2Vector3<float> points2outerHit((outerHit.X() - circleCenter.X()),
                                       (outerHit.Y() - circleCenter.Y()),
                                       (outerHit.Z() - circleCenter.Z()));
      B2Vector3<float> points2centerHit((centerHit.X() - circleCenter.X()),
                                        (centerHit.Y() - circleCenter.Y()),
                                        (centerHit.Z() - circleCenter.Z()));
      B2Vector3<float> points2innerHit((innerHit.X() - circleCenter.X()),
                                       (innerHit.Y() - circleCenter.Y()),
                                       (innerHit.Z() - circleCenter.Z()));

      float alfaOC = Helper::fullAngle2D(points2outerHit, points2centerHit);
      float alfaCI = Helper::fullAngle2D(points2centerHit, points2innerHit);

      // equals to alfaAB/dZAB and alfaBC/dZBC, but this solution here can not produce a division by zero:
      return (alfaOC * (centerHit.Z() - innerHit.Z())) - (alfaCI * (outerHit.Z() - centerHit.Z()));
    } // return unit: radians*cm
  };

}
