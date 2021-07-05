/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/paths/BaseCDCPathFilter.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  /// Return the size of the path
  class SizeCDCPathFilter : public BaseCDCPathFilter {
  public:
    /// Main function: return the size of the path
    TrackFindingCDC::Weight operator()(const BaseCDCPathFilter::Object& path) final {
      return path.size();
    }
  };
}
