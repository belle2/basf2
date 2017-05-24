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
  class SimpleCDCTrackSpacePointCombinationFilter : public BaseCDCTrackSpacePointCombinationFilter {
  public:
    void exposeParameters(ModuleParamList* moduleParamList,
                          const std::string& prefix) final;

    /// Test if the parameters are valid
    void initialize() final;

    /// Set the cached B field
    void beginRun() final;

    TrackFindingCDC::Weight operator()(const BaseCDCTrackSpacePointCombinationFilter::Object& currentState) final;

  private:
    std::vector<double> m_param_maximumHelixChi2XYZ {10000, 10000, 10000, 10000};
    std::vector<double> m_param_maximumChi2XY {100, 100, 100, 100};
    std::vector<double> m_param_maximumChi2 {10000, 10000, 10000, 10000};

    /// Parameter: make hit jumps possible (missing hits on a layer)
    int m_param_hitJumpingUpTo = 1;

    /// Cache for the B field at the IP
    double m_cachedBField;
  };
}
