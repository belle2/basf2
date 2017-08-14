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
  class SimpleCKFCDCToSVDStateFilter : public SimpleCKFSpacePointFilter {
  public:
    SimpleCKFCDCToSVDStateFilter() : SimpleCKFSpacePointFilter()
    {
      m_param_maximumHelixChi2XYZ = {10000, 10000, 10000, 10000};
      m_param_maximumChi2XY = {100, 100, 100, 100};
      m_param_maximumChi2 = {10000, 10000, 10000, 10000};
    }

    /// Return NAN, if this state should not be used
    TrackFindingCDC::Weight operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState) final;
  private:
    /// Maximal values for the first chi2 check based on the helix calculation
    std::vector<double> m_param_maximumHelixChi2XYZ;
    /// Maximal values for the second chi2 check based on an extrapolation
    std::vector<double> m_param_maximumChi2XY;
    /// Maximal values for the third chi2 check based on a Kalman fit
    std::vector<double> m_param_maximumChi2;
  };
}
