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
#include <tracking/trackFindingVXD/FilterTools/SelectionVariableHelperFunctions.h>
#include <math.h>


// in fw:
#include <framework/logging/Logger.h>

namespace Belle2 {

  /** calculates the angle between the hits/vectors (3D), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use Angle3DFull instead).
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class Angle3DSimple : public SelectionVariable< PointType , float > {
  public:

    /** calculates the angle between the hits/vectors (3D), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use Angle3DFull instead) */
    static float value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      using namespace SelVarHelper;

      B2Vector3<float> outerVector = doAMinusB<PointType, float>(outerHit, centerHit);
      B2Vector3<float> innerVector = doAMinusB<PointType, float>(centerHit, innerHit);

      // fullCalc would be acos(m_vecAB.Dot(m_vecBC) / m_vecAB.Mag()*m_vecBC.Mag()), but here time-consuming parts have been neglected
      float result = outerVector.Dot(innerVector) / (outerVector.Mag2() * innerVector.Mag2());
      return
        (std::isnan(result) || std::isinf(result)) ? 0 : result;
    }
  };

}
