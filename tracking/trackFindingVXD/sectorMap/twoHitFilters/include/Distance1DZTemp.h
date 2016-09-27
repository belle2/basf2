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
#include <math.h>

namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the  distance between two hits in 1D on the Z-axis.
   *
   * (please note that this is defined for positive and negative values) */
  template <typename PointType >
  class Distance1DZTemplate : public SelectionVariable< PointType , double > {
  public:

    /** calculates the distance between the hits in z (1D), returning unit: cm */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {
      return
        outerHit.Z() - innerHit.Z();
    }
  };

  /** typedef for common usage with SpacePoints */
//   typedef Distance1DZTemplate<SpacePoint> SPDistance1DZ;

}
