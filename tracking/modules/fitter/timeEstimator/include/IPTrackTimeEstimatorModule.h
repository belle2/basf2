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

  /** Base Module estimating the track time of RecoTracks - before or after the fit.
   * It extrapolates the helix/state to the IP and corrects for the traveled time.  */
  class IPTrackTimeEstimatorModule : public BaseTrackTimeEstimatorModule {
  private:
    /// Extrapolate the first measurement to the center of the IP.
    double estimateFlightLengthUsingFittedInformation(genfit::MeasuredStateOnPlane& measuredStateOnPlane) const override;

    /// Calculate the flight length from the origin or the perigee to the start position of the track.
    double estimateFlightLengthUsingSeedInformation(const RecoTrack& recoTrack) const override;
  };
}
