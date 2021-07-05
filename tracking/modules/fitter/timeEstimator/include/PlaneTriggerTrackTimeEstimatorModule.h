/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/modules/fitter/timeEstimator/BaseTrackTimeEstimatorModule.h>

namespace Belle2 {

  /** Module estimating the track time of RecoTracks - before or after the fit.
   * It extrapolates the helix/state to the given plane and corrects for the traveled time.  */
  class PlaneTriggerTrackTimeEstimatorModule : public BaseTrackTimeEstimatorModule {

  public:
    /** Initilialize the module parameters. */
    PlaneTriggerTrackTimeEstimatorModule();

  private:
    /// 3-Position of the plane of the trigger.
    std::vector<double> m_param_triggerPlanePosition = {0, 0, 0};
    /// 3-Normal direction of the plane of the trigger.
    std::vector<double> m_param_triggerPlaneNormalDirection = {1, 0, 0};

    /// Estimate the flight length to the given plane using the extrapolation of the fit.
    double estimateFlightLengthUsingFittedInformation(genfit::MeasuredStateOnPlane& measuredStateOnPlane) const override;
    /// Estimate the flight length using only the tracking seeds.
    double estimateFlightLengthUsingSeedInformation(const RecoTrack& recoTrack) const override;
  };
}
