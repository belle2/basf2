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

  /** This is the specialization for SpacePoints with returning floats, where value calculates the squared distance between two hits in 2D on the X-Y-plane */
  class Distance2DXYSquared : public SelectionVariable< SpacePoint , float > {
  public:

    /** calculates the squared distance between the hits (2D on the X-Y-plane), returning unit: cm^2 for speed optimization */
    static float value(const SpacePoint& hitA, const SpacePoint& hitB) {
      return
        std::pow(hitA.getPosition().X() - hitB.getPosition().X() , 2) +
        std::pow(hitA.getPosition().Y() - hitB.getPosition().Y() , 2);
    }
  };

}
