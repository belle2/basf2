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

#define HELIXPARAMETERFIT_NAME HelixParameterFit

namespace Belle2 {

  /** calculates the helixparameter describing the deviation in z per unit angle, returning unit: none.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class HELIXPARAMETERFIT_NAME : public SelectionVariable< PointType , 3, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(HELIXPARAMETERFIT_NAME);


    /** calculates the helixparameter describing the deviation in z per unit angle, returning unit: none */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {

      B2Vector3D cCenter = CircleCenterXY<PointType>::value(outerHit, centerHit, innerHit);

      B2Vector3D vecOuter2cC(outerHit.X() - cCenter.X(), outerHit.Y() - cCenter.Y(), outerHit.Z() - cCenter.Z());
      B2Vector3D vecCenter2cC(centerHit.X() - cCenter.X(), centerHit.Y() - cCenter.Y(), centerHit.Z() - cCenter.Z());
      B2Vector3D vecInner2cC(innerHit.X() - cCenter.X(), innerHit.Y() - cCenter.Y(), innerHit.Z() - cCenter.Z());

      double alfaAB = CosDirectionXY<B2Vector3D>::value(vecOuter2cC, vecCenter2cC);
      double alfaBC = CosDirectionXY<B2Vector3D>::value(vecCenter2cC, vecInner2cC);

      // real calculation: ratio is (m_vecij[2] = deltaZ): alfaAB/deltaZab : alfaBC/deltaZbc, the following equation saves two times '/'
      double result = (alfaAB * double(centerHit.Z() - innerHit.Z())) / (alfaBC * double(outerHit.Z() - centerHit.Z()));

      return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
    } // return unit: none
  };

}
