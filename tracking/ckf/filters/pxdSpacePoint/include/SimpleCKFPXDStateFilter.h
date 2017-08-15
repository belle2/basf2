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
    TrackFindingCDC::Weight operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState) final;

  private:
    using MaximalValueArray = double[2][3];

    static constexpr const MaximalValueArray m_param_maximumHelixDistanceXY = {
      {2 * 0.034200, 2 * 0.129300, 2 * 0.497570}, // not used
      {2 * 0.396960, 2 * 1.480930, 2 * 2.709820}
    };

    static constexpr const MaximalValueArray m_param_maximumHelixDistance = {
      {2 * 0.086890, 2 * 0.295830, 2 * 1.959520},
      {2 * 8.010050, 2 * 82.159590, 2 * 71.357270} // not used
    };

    static constexpr const MaximalValueArray m_param_maximumResidualOverSigma = {
      {2 * 34.889460, 2 * 89.291220, 2 * 215.008860}, // not used
      {2 * 47.673060, 2 * 174.674040, 2 * 163.161100}
    };

    static constexpr const MaximalValueArray m_param_maximumResidual = {
      {2 * 0.087460, 2 * 0.219350, 2 * 2.115020},
      {2 * 8.009960, 2 * 75.095190, 2 * 71.185920} // not used
    };

    static constexpr const MaximalValueArray m_param_maximumChi2 = {
      {2 * 1015.621520, 2 * 24882.423280, 2 * 62956.932580},
      {2 * 2035.318930, 2 * 29494.953340, 2 * 42998.393910}
    };

    KalmanUpdateFitter m_fitter;
  };
}
