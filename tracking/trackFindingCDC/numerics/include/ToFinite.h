/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
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

    /// Helper function to replace a non-finite value with a defined value
    inline double toFinite(double value, double replacement)
    {
      return std::isfinite(value) ? value : replacement;
    }
  }
}
