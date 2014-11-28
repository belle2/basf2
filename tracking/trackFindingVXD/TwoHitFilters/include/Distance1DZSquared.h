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

  /** This is the specialization for SpacePoints with returning floats, where value calculates the squared distance between two hits in 1D on the Z-axis */
  class Distance1DZSquared : public SelectionVariable< SpacePoint , float > {
  public:

    /** calculates the squared distance between the hits in Z (in 1D), returning unit: cm^2 */
    static float value(const SpacePoint& outerHit, const SpacePoint& innerHit) {
      return
        std::pow(outerHit.Z() - innerHit.Z(), 2);
    }
  };

}
