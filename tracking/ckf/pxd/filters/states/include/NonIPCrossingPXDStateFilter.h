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

#include <tracking/ckf/pxd/filters/states/AllPXDStateFilter.h>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Simple filter which does only allow states, which are on the same side of the track (in respective to the IP).
   *
   * It does this by looking onto the difference between the arc length 2D of the hit
   * and the arc length 2D of the origin, which should be negative (or positive) when the extrapolation is
   * going downwards (upwards).
   */
  class NonIPCrossingPXDStateFilter : public AllPXDStateFilter {
  public:
    /// Return NAN or 1.
    TrackFindingCDC::Weight operator()(const AllPXDStateFilter::Object& pair) final;

    /// Expose the direction parameter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Which extrapolation direction should be tested.
    double m_param_direction = 1;
  };
}
