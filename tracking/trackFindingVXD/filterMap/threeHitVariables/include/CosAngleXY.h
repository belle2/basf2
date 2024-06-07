/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <framework/geometry/B2Vector3.h>
#include <cmath>

#define COSANGLEXY_NAME CosAngleXY

namespace Belle2 {

  /** calculates the angle between the hits/vectors (XY),
   * returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleXYFull instead):
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class COSANGLEXY_NAME : public SelectionVariable< PointType, 3, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(COSANGLEXY_NAME);


    /** calculates the angle between the hits/vectors (XY),
     * returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleXYFull instead) */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {

      B2Vector3D outerVector(outerHit.X() - centerHit.X(), outerHit.Y() - centerHit.Y(), 0.);
      B2Vector3D innerVector(centerHit.X() - innerHit.X(), centerHit.Y() - innerHit.Y(), 0.);

      double result = (outerVector.X() * innerVector.X() + outerVector.Y() * innerVector.Y()) / (outerVector.Perp() *
                      innerVector.Perp());

      return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
    } // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleXYFull instead)
  };

}
