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

  /** calculates the helixparameter describing the deviation in z per unit angle, returning unit: none.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class HelixParameterFit : public SelectionVariable< PointType , double > {
  public:

    /** calculates the helixparameter describing the deviation in z per unit angle, returning unit: none */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D cCenter = Helper::calcCircleCenter(outerHit, centerHit, innerHit);

      B2Vector3D vecOuter2cC(outerHit.X() - cCenter.X(), outerHit.Y() - cCenter.Y(), outerHit.Z() - cCenter.Z());
      B2Vector3D vecCenter2cC(centerHit.X() - cCenter.X(), centerHit.Y() - cCenter.Y(), centerHit.Z() - cCenter.Z());
      B2Vector3D vecInner2cC(innerHit.X() - cCenter.X(), innerHit.Y() - cCenter.Y(), innerHit.Z() - cCenter.Z());
      double alfaAB = Helper::calcAngle2D(vecOuter2cC, vecCenter2cC);
      double alfaBC = Helper::calcAngle2D(vecCenter2cC, vecInner2cC);

      // real calculation: ratio is (m_vecij[2] = deltaZ): alfaAB/deltaZab : alfaBC/deltaZbc, the following equation saves two times '/'
      double result = (alfaAB * double(centerHit.Z() - innerHit.Z())) / (alfaBC * double(outerHit.Z() - centerHit.Z()));

      return Helper::checkValid(result);
    } // return unit: none
  };

}
