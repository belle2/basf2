/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <math.h>

#define DISTANCE2DXYSQUARED_NAME Distance2DXYSquared

namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the squared distance between two hits in 2D on the X-Y-plane */
  template <typename PointType>
  class DISTANCE2DXYSQUARED_NAME : public SelectionVariable< PointType, 2, double > {
  public:

    /** is expanded as "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DISTANCE2DXYSQUARED_NAME);

    /** calculates the squared distance between the hits (2D on the X-Y-plane), returning unit: cm^2 for speed optimization */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {
      return
        std::pow(outerHit.X() - innerHit.X(), 2) +
        std::pow(outerHit.Y() - innerHit.Y(), 2);
    }
  };

}
