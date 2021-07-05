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
