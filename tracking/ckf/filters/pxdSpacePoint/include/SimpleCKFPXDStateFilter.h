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
#include <tracking/ckf/findlets/base/KalmanUpdateFitter.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  class SimpleCKFPXDStateFilter : public SimpleCKFSpacePointFilter {
  public:
    /// Return NAN, if this state should not be used
    TrackFindingCDC::Weight operator()(const BaseCKFCDCToSpacePointStateFilter::Object& currentState) final;

  private:
    /// Shortcut for a 2x3 array.
    using MaximalValueArray = double[2][3];

    /// Maximum distance calculated with helix extrapolation in filter 1. Numbers calculated on MC.
    static constexpr const MaximalValueArray m_param_maximumHelixDistanceXY = {
      {0.295060, 0.295060, 0.295060},
      {0.295060, 0.295060, 0.295060}
    };

    /// Maximum distance calculated with normal extrapolation in filter 2. Numbers calculated on MC.
    static constexpr const MaximalValueArray m_param_maximumResidual = {
      {0.679690, 0.677690, 0.427060},
      {1.006930, 0.926120, 0.687030}
    };

    /// Maximum chi^2 in filter 3. Numbers calculated on MC.
    static constexpr const MaximalValueArray m_param_maximumChi2 = {
      {10, 50, 125},
      {20, 60, 85}
    };

    /// Fitter used for calculating chi2
    KalmanUpdateFitter m_fitter;
  };
}
