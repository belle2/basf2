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

#include <boost/math/special_functions/fpclassify.hpp> // isnan and isinf

#include <math.h>

namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the  slope in R-Z for a given pair of hits.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  class SlopeRZ : public SelectionVariable< SpacePoint , float > {
  public:

    /** calculates the distance between the hits in z (1D), returning unit: cm */
    static float value(const SpacePoint& outerHit, const SpacePoint& innerHit) {
      float result = atan(
                       std::sqrt(
                         std::pow((outerHit.X() - innerHit.X()), 2)
                         + std::pow((outerHit.Y() - innerHit.Y()), 2)
                       ) / (outerHit.Z() - innerHit.Z())
                     );
      return
        (std::isnan(result) || std::isinf(result)) ? 0 : result;
    }
  };

}
