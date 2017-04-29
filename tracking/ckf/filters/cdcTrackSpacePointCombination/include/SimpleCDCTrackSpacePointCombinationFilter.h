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

    void initialize() override;

    TrackFindingCDC::Weight operator()(const BaseCDCTrackSpacePointCombinationFilter::Object& currentState) final;

  private:
    std::vector<double> m_param_maximumXYDistance {10, 10, 10, 10};
    std::vector<double> m_param_maximumDistance {20, 20, 20, 20};
    std::vector<double> m_param_maximumChi2Difference {100, 100, 100, 100};
  };
}
