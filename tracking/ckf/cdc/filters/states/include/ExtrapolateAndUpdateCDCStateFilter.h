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

#include <tracking/ckf/cdc/filters/states/BaseCDCStateFilter.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>

#include <tracking/ckf/general/utilities/Advancer.h>
#include <tracking/ckf/general/utilities/KalmanStepper.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <string>

namespace Belle2 {
  class ModuleParamList;

  /// A very extrapolateAndUpdate filter for all CDC states.
  class ExtrapolateAndUpdateCDCStateFilter : public BaseCDCStateFilter {
  public:
    ExtrapolateAndUpdateCDCStateFilter();

    TrackFindingCDC::Weight operator()(const BaseCDCStateFilter::Object& pair) final;

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    Advancer m_extrapolator;
    KalmanStepper<1> m_updater;
  };
}
