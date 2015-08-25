/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// An additive measure of quality
    typedef double Weight;

    /// Constant for the highest possible weight
    extern const Weight HIGHEST_WEIGHT;

    /// Constant for the lowest possible weight
    extern const Weight LOWEST_WEIGHT;

  } // namespace TrackFindingCDC

} // namespace Belle2
