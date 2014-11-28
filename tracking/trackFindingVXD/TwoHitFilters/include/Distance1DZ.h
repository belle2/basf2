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

#include "tracking/trackFindingVXD/FilterTools/SelectionVariable.h"

#include <tracking/spacePointCreation/SpacePoint.h>

#include <math.h>

namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the  distance between two hits in 1D on the Z-axis.
   *
   * (please note that this is defined for positive and negative values) */
  class Distance1DZ : public SelectionVariable< SpacePoint , float > {
  public:

    /** calculates the distance between the hits in z (1D), returning unit: cm */
    static float value(const SpacePoint& outerHit, const SpacePoint& innerHit) {
      return
        outerHit.Z() - innerHit.Z();
    }
  };

}
