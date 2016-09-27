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

  /** calculates the angle between the hits/vectors (3D), returning unit: angle in degrees.
   *
   * calculating with double precission no matter the resolution of the output.
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class Angle3DFull : public SelectionVariable< PointType, double > {
  public:

    /** calculates the angle between the hits/vectors (3D), returning unit: angle in degrees */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D outerVector = Helper::doAMinusB(outerHit, centerHit);
      B2Vector3D innerVector = Helper::doAMinusB(centerHit, innerHit);

      double result = acos(outerVector.Dot(innerVector) / (outerVector.Mag() * innerVector.Mag())); // 0-pi
      result = (result * (180. / M_PI));
      return Helper::checkValid(result);
    }
  };

}
