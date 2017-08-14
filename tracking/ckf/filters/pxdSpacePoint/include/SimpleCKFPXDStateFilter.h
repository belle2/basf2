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
  class SimpleCKFPXDStateFilter : public SimpleCKFSpacePointFilter {
  public:
    /// Return NAN, if this state should not be used
    TrackFindingCDC::Weight operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState) final;

  private:
    static constexpr const double m_param_maximumHelixDistanceXY[2][3] = {
      {2 * 0.034200, 2 * 0.129300, 2 * 0.497570},
      {2 * 0.396960, 2 * 1.480930, 2 * 2.709820}
    };

    static constexpr const double m_param_maximumDistanceXY[2][3] = {
      {2 * 0.036950, 2 * 0.139020, 2 * 0.791550}, // not used
      {2 * 0.440430, 2 * 1.218590, 2 * 2.542560}
    };

    static constexpr const double m_param_maximumDistance[2][3] = {
      {2 * 0.087460, 2 * 0.219350, 2 * 2.115020},
      {2 * 8.009960, 2 * 75.095190, 2 * 71.185920}  // not used
    };

    static constexpr const double m_param_maximumChi2[2][3] = {
      {2 * 1015.621520, 2 * 24882.423280, 2 * 62956.932580},
      {2 * 2035.318930, 2 * 29494.953340, 2 * 42998.393910}
    };
  };
}
