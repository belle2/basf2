/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
