/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
