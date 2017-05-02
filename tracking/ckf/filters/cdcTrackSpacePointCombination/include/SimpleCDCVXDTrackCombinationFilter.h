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

#include <tracking/ckf/filters/cdcTrackSpacePointCombination/BaseCDCTrackSpacePointCombinationFilter.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  class SimpleCDCVXDTrackCombinationFilter : public BaseCDCVXDTrackCombinationFilter {
  public:
    TrackFindingCDC::Weight operator()(const BaseCDCVXDTrackCombinationFilter::Object& currentState) final;
  };
}
