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
  class SimpleCKFCDCToSVDStateFilter : public SimpleCKFSpacePointFilter {
  public:
    /// Return NAN, if this state should not be used
    TrackFindingCDC::Weight operator()(const BaseCKFCDCToSpacePointStateFilter::Object& currentState) final;

  private:
    /// Shortcut for a 4x3 array.
    using MaximalValueArray = double[4][3];

    /// Maximum distance calculated with helix extrapolation in filter 1. Numbers calculated on MC.
    static constexpr const MaximalValueArray m_param_maximumHelixDistance = {
      // high pt, middle pt, low pt
      {1.38532, 1.44346, 1.53386}, // layer 3
      {1.96701, 2.17819, 2.19784}, // layer 4
      {2.33064, 2.67768, 2.67174}, // layer 5
      {6, 6, 6}, // layer 6
    };

    /// Maximum distance calculated with normal extrapolation in filter 3. Numbers calculated on MC.
    static constexpr const MaximalValueArray m_param_maximumResidual = {
      {1.52328, 1.57714, 1.63069}, // layer 3
      {1.61301, 1.78301, 1.75973}, // layer 4
      {1.85188, 2.19571, 2.38167}, // layer 5
      {5.5, 5, 5}, // layer 6
    };

    /// Maximum distance calculated with normal extrapolation in filter 2. Numbers calculated on MC.
    static constexpr const MaximalValueArray m_param_maximumResidual2 = {
      {0.69713, 0.39872, 0.41854}, // layer 3
      {0.72737, 0.40816, 0.53138}, // layer 4
      {0.35624, 0.12846, 0.22516}, // layer 5
      {0.00975, 0.06432, 0.5969}, // layer 6
    };

    /// Fitter used for calculating chi2
    KalmanUpdateFitter m_fitter;
  };
}
