/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/CosDirectionXY.h>
#include <framework/geometry/B2Vector3.h>
#include <cmath>

#define ANGLEXYFULL_NAME AngleXYFull

namespace Belle2 {

  /** calculates the angle between the hits/vectors (XY), returning unit: angle in degrees.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class ANGLEXYFULL_NAME : public SelectionVariable< PointType, 3, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(ANGLEXYFULL_NAME);

    /** calculates the angle between the hits/vectors (XY), returning unit: angle in degrees */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {

      B2Vector3D outerVector(outerHit.X() - centerHit.X(), outerHit.Y() - centerHit.Y(), outerHit.Z() - centerHit.Z());
      B2Vector3D innerVector(centerHit.X() - innerHit.X(), centerHit.Y() - innerHit.Y(), centerHit.Z() - innerHit.Z());

      double result = acos(CosDirectionXY<B2Vector3D>::value(outerVector, innerVector));   // 0-pi
      result *= double(180. / M_PI); // do conversion into degree
      return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
    } // return unit: ° (0 - 180°)
  };

}
