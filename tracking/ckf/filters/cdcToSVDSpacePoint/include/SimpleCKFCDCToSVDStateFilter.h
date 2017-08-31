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
    TrackFindingCDC::Weight operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState) final;

  private:
    using MaximalValueArray = double[4][3];

    static constexpr const MaximalValueArray m_param_maximumHelixDistance = {
      // high pt, middle pt, low pt
      {0.15733, 3.40249, 10.95084}, // layer 3
      {0.22804, 0.93842, 8.78157}, // layer 4
      {0.48812, 4.82704, 16.30189}, // layer 5
      {12.14745, 109.19225, 426.43899}, // layer 6
    };

    static constexpr const MaximalValueArray m_param_maximumResidual = {
      {1.12139, 1.12533, 12.03218}, // layer 3
      {1.00493, 1.24574, 10.51225}, // layer 4
      {1.14461, 4.0116, 20.48132}, // layer 5
      {6.49328, 100.7814, 176.81082}, // layer 6
    };

    static constexpr const MaximalValueArray m_param_maximumResidual2 = {
      {0.0521, 0.08309, 2.07406}, // layer 3
      {0.06964, 0.12137, 4.80165}, // layer 4
      {0.05506, 0.1015, 0.5402}, // layer 5
      {0.00975, 0.06432, 0.5969}, // layer 6
    };

    static constexpr const MaximalValueArray m_param_maximumChi2 = {
      {1000, 1000, 1000},  // layer 3
      {1000, 1000, 1000},  // layer 4
      {1000, 1000, 1000},  // layer 5
      {100000, 100000, 100000},  // layer 6
    };

    KalmanUpdateFitter m_fitter;
  };
}
