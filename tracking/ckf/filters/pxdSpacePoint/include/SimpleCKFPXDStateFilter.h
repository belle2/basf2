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

#include <tracking/ckf/filters/cdcToSpacePoint/state/SimpleCKFSpacePointFilter.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  class SimpleCKFPXDStateFilter : public SimpleCKFSpacePointFilter {
  public:
    /// Return NAN, if this state should not be used
    TrackFindingCDC::Weight operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState) final;
  };
}
