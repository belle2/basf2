/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/states/BaseCDCStateFilter.h>

#include <tracking/ckf/general/utilities/Advancer.h>
#include <tracking/ckf/general/utilities/KalmanStepper.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <string>

namespace Belle2 {
  class ModuleParamList;

  /// An extrapolateAndUpdate filter for all CDC states.
  class ExtrapolateAndUpdateCDCStateFilter : public BaseCDCStateFilter {
  public:
    ExtrapolateAndUpdateCDCStateFilter();

    /// Extrapolate along the path (pair.first) to the CDC wireHit-state (pair.second). Return 1/chi2 if Ok, NAN otherwise.
    TrackFindingCDC::Weight operator()(const BaseCDCStateFilter::Object& pair) final;

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Kalman filter extrapolator
    Advancer m_extrapolator;

    /// Kalman filter updater
    KalmanStepper<1> m_updater;
  };
}
