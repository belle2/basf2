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
