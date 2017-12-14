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
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/CosDirectionXY.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

#define DELTASOVERZ_NAME DeltaSoverZ

namespace Belle2 {

  /** calculates the helixparameter describing the deviation in arc length per unit in z.
   * returning unit: radians*cm. */
  template <typename PointType >
  class DELTASOVERZ_NAME : public SelectionVariable< PointType , 3, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DELTASOVERZ_NAME);


    /** calculates the helixparameter describing the deviation in arc length per unit in z.
     * returning unit: radians*cm */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {

      B2Vector3D circleCenter = CircleCenterXY<PointType>::value(outerHit, centerHit, innerHit);
      B2Vector3D points2outerHit((outerHit.X() - circleCenter.X()),
                                 (outerHit.Y() - circleCenter.Y()),
                                 (outerHit.Z() - circleCenter.Z()));
      B2Vector3D points2centerHit((centerHit.X() - circleCenter.X()),
                                  (centerHit.Y() - circleCenter.Y()),
                                  (centerHit.Z() - circleCenter.Z()));
      B2Vector3D points2innerHit((innerHit.X() - circleCenter.X()),
                                 (innerHit.Y() - circleCenter.Y()),
                                 (innerHit.Z() - circleCenter.Z()));

      double alfaOC = acos(CosDirectionXY<B2Vector3D>::value(points2outerHit, points2centerHit));
      double alfaCI = acos(CosDirectionXY<B2Vector3D>::value(points2centerHit, points2innerHit));

      // equals to alfaAB/dZAB and alfaBC/dZBC, but this solution here can not produce a division by zero:
      return (alfaOC * double(centerHit.Z() - innerHit.Z())) - (alfaCI * double(outerHit.Z() - centerHit.Z()));
    } // return unit: radians*cm
  };

}
