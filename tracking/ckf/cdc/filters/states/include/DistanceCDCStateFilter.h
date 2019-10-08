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

#include <tracking/ckf/cdc/filters/states/BaseCDCStateFilter.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  /// Give a weight based on the distance from the hit to the path
  class DistanceCDCStateFilter : public BaseCDCStateFilter {
  public:
    /// Return the weight based on the distance
    TrackFindingCDC::Weight operator()(const BaseCDCStateFilter::Object& pair) final;
    /// Expose the parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;
  private:
    /// Cut value for maximal distance
    double m_maximalDistance = 1000.;
  };
}
