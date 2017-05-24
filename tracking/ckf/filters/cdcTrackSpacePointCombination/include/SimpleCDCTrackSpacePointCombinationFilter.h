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
    std::vector<double> m_param_maximumHelixChi2XYZ {10000, 10000, 10000, 10000};
    std::vector<double> m_param_maximumChi2XY {100, 100, 100, 100};
    std::vector<double> m_param_maximumChi2 {10000, 10000, 10000, 10000};

    /// Object pool for mSoP position
    TVector3 m_position;
    /// Object pool for mSoP momentum
    TVector3 m_momentum;
    /// Object pool for mSoP covariance matrix
    TMatrixDSym m_cov;
  };
}
