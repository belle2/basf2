/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
