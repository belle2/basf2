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

#include <tracking/ckf/svd/filters/states/BaseSVDStateFilter.h>
#include <tracking/ckf/svd/utilities/SVDKalmanStepper.h>
#include <framework/gearbox/Unit.h>

namespace Belle2 {
  /// A very simple filter for all space points.
  class ResidualSVDStateFilter : public BaseSVDStateFilter {
  public:
    /// Return 1/residual and cut at the maximum residual.
    TrackFindingCDC::Weight operator()(const BaseSVDStateFilter::Object& pair) final;

    /// Expose the maximal residual
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Kalman Stepper to calculate the residual
    SVDKalmanStepper m_kalmanStepper;

    /// Parameter: maximal residual
    double m_param_maximalResidual = 2 * Unit::cm;
  };
}
