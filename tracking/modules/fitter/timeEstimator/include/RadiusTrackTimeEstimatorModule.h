/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/fitter/timeEstimator/BaseTrackTimeEstimatorModule.h>

namespace Belle2 {

  /** Module estimating the track time of RecoTracks - before or after the fit.
   * It extrapolates the helix/state to the radius and corrects for the traveled time.  */
  class RadiusTrackTimeEstimatorModule : public BaseTrackTimeEstimatorModule {

  public:
    /** Initialize the module parameters. */
    RadiusTrackTimeEstimatorModule();

  private:
    /// Radius used for extrapolation. Please be aware that if the RecoTrack does not reach this radius, the results are wrong.
    double m_param_radiusForExtrapolation = 0;

    /// Extrapolate the measured state on plane to the given radius.
    double estimateFlightLengthUsingFittedInformation(genfit::MeasuredStateOnPlane& measuredStateOnPlane) const override;

    /// Use some helix functionality to calculate the distance to given the radius.
    double estimateFlightLengthUsingSeedInformation(const RecoTrack& recoTrack) const override;
  };
}
