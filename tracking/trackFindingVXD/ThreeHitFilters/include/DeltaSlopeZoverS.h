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

  /** compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class DeltaSlopeZoverS : public SelectionVariable< PointType , float > {
  public:

    /** compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane. */
    static float value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, float> Helper;

      B2Vector3<float> cCenter = Helper::calcCircleCenter(outerHit, centerHit, innerHit);
      float circleRadius = Helper::calcRadius(outerHit, centerHit, innerHit, cCenter);
      B2Vector3<float> vecOuter2cC((outerHit.X() - cCenter.X()),
                                   (outerHit.Y() - cCenter.Y()),
                                   (outerHit.Z() - cCenter.Z()));
      B2Vector3<float> vecCenter2cC((centerHit.X() - cCenter.X()),
                                    (centerHit.Y() - cCenter.Y()),
                                    (centerHit.Z() - cCenter.Z()));
      B2Vector3<float> vecInner2cC((innerHit.X() - cCenter.X()),
                                   (innerHit.Y() - cCenter.Y()),
                                   (innerHit.Z() - cCenter.Z()));

      float alfaOCr = Helper::fullAngle2D(vecOuter2cC, vecCenter2cC) * circleRadius;
      float alfaCIr = Helper::fullAngle2D(vecCenter2cC, vecInner2cC) * circleRadius;

      // Beware of z>r!:
      float result = (asin((outerHit.Z() - centerHit.Z()) / alfaOCr)) - asin((centerHit.Z() - innerHit.Z()) / alfaCIr);

      return Helper::checkValid(result);
    }
  };

}



/** compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane. */
// double calcDeltaSlopeZOverS()
// {
//   checkCalcRadius();
//
//   TVector3 points2hitA = m_hitA - m_centerABC;
//   TVector3 points2hitB = m_hitB - m_centerABC;
//   TVector3 points2hitC = m_hitC - m_centerABC;
//   double alfaABr = fullAngle2D(points2hitA, points2hitB) * m_radius;
//   double alfaBCr = fullAngle2D(points2hitB, points2hitC) * m_radius;
//
//   return filterNan((asin(m_vecAB[2] / alfaABr)) - asin(m_vecBC[2] / alfaBCr)); // Beware of z>r!
// }
