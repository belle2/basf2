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

  /** calculates the angle between the hits/vectors (3D), returning unit: angle in degrees.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class Angle3DFull : public SelectionVariable< PointType , float > {
  public:

    /** calculates the angle between the hits/vectors (3D), returning unit: angle in degrees */
    static float value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      using namespace SelVarHelper;

      B2Vector3<float> outerVector = doAMinusB<PointType, float>(outerHit, centerHit);
      B2Vector3<float> innerVector = doAMinusB<PointType, float>(centerHit, innerHit);

      double result = acos(outerVector.Dot(innerVector) / (outerVector.Mag() * innerVector.Mag())); // 0-pi
      result = (result * (180. / M_PI));
      return (std::isnan(result) || std::isinf(result)) ? 0 : result;
    }
  };

}
