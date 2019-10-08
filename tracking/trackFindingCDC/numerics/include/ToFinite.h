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
#include <RtypesCore.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Helper function to replace a non-finite value with a defined value
    inline Float_t toFinite(Float_t value, Float_t replacement)
    {
      return std::isfinite(value) ? value : replacement;
    }
  }
}
