/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <TDecompChol.h>
#include <TDecompSVD.h>
#include <TVectorD.h>

#include <vector>

namespace Belle2 {
  /**
   * Helper class to perform all kind of track extrapolations using the methods from arXiv:0810.2241.
   *
   * The event time is calculated as one would calculate an alignment
   * parameter.  We follow the prescription in arXiv:0810.2241
   * [physics.ins-det] to build the full covariance matrix for the
   * Kalman-fitted track (DAF counts as Kalman).  We then evaluate the
   * change in event time that minimizes the sum of chi^2s of the tracks
   * following the same procedure that is decribed in loc.cit.
   *
   * Unlike the case of alignment, we only have one free parameter (time),
   * and therefore there are no large matrices to invert.  The necessary
   * timeshift is calculated from the estimated derivatives of chi2^2 in
   * the linear approximation as
   *                dchi^2   /d^2chi^2
   *   delta t = - -------- / -------- .
   *                  dt   /    dt^2
   * Here division replaces a matrix inverse (this is loc.cit. Eq. (8)).
   *
   * Time-dependence of the fit result comes from the CDC hits, where a
   * later time of particle passage corresponds to a greater drift
   * circle.
   *
   */
  class TimeExtractionUtils {
  public:

    static std::vector<RecoTrack*> selectTracksForTimeExtraction(std::vector<RecoTrack*> const& tracks,
        unsigned int minNumberCDCHits = 20, unsigned int maximumNumberOfTracks = std::numeric_limits<unsigned int>::max(),
        double minimumTrackPt = 0.35)
    {
      for (RecoTrack* rt : tracks) {
        B2DEBUG(50, "Got RecoTrack for selection with " << rt->getNumberOfCDCHits() << " CDC Hits");
      }

      auto filteredTracks = TrackFindingCDC::copy_if(tracks, [minNumberCDCHits, minimumTrackPt](RecoTrack * rt) {
        return (rt->getNumberOfCDCHits() >= minNumberCDCHits) && (rt->getMomentumSeed().Mag() >= minimumTrackPt);
      });

      // sort by the amount of CDC hits
      std::sort(filteredTracks.begin(), filteredTracks.end(),
                // this lambda will sort in reverse order, meaning the tracks with the most CDC hits first
      [](RecoTrack * a, RecoTrack * b) { return a->getNumberOfCDCHits() > b->getNumberOfCDCHits(); });

      // limit to the maximum number of tracks
      filteredTracks.resize(std::min((unsigned int)filteredTracks.size(), maximumNumberOfTracks));

      B2DEBUG(50, "Limited number of selected tracks: " << filteredTracks.size());
      for (auto tr : filteredTracks) {
        B2DEBUG(50, "Selected track with " << tr->getNumberOfCDCHits() << " CDC Hits");
      }

      return filteredTracks;
    }

    /**
     * Main function of this class: do one time extraction step by calculating the
     * derivatives of chi^2 to the global event time for minimizing chi^2.
     *
     * The event time is not applied neither to the track nor to the global EventT0, but only returned.
     */
    static double extractTime(const RecoTrack& recoTrack)
    {
      const auto& extractedDerivativePair = getChi2Derivatives(recoTrack);
      return extractedDerivativePair.first / extractedDerivativePair.second;
    }

    /**
     * Extract the derivatives d chi^2 / d alpha and d^2 chi^2 / (d alpha)^2 from the reco track
     * by calculating the full residual covariance matrix, where alpha is the global event time.
     *
     * The two derivatives are returned as a pair.
     */
    static std::pair<double, double> getChi2Derivatives(const RecoTrack& recoTrack);

    /**
     * Get a list of dimensions for each measurement in the reco track. Needed for the derivatives. E.g. the residuals
     * vector is as large as the sum of the returned vector here.
     * @param recoTrack The reco track from which the measurements should be taken.
     * @return An std::vector with the number of dimensions for each measurement. Has as many entries as there are
     * measurements in the reco track.
     */
    static std::vector<int> getMeasurementDimensions(const RecoTrack& recoTrack);

    /**
     * Build the full covariance matrix of the given reco track and fills it into the given TMatrixDSym.
     * Needed for the derivatives.
     *
     * The full covariance contains the covariances of the different states of the reco track at different hits,
     * this means is connects the calculates state at one measurement to one at another measurement. This is why it
     * has the dimension number of hits * dimensionality of the track state. The diagonal blocks are filled with
     * the covariance matrices at a single measurement whereas the off diagonal elements connect different measurements.
     *
     * The off-diagnal elements are calculated, by either using one prediction step forward (because the two hits
     * are neighbors), by using the fact that if thei are not neighbors they can be connected by a finite number of
     * forward predictions which breaks this down to the first case and some multiplications again
     * or by using symmetry aspects (e.g. no background prediction is needed).
     *
     * The notation follows 0810.2241.
     *
     * Returns false if something strange happened, otherwise true.
     */
    static bool buildFullCovarianceMatrix(const RecoTrack& recoTrack,
                                          TMatrixDSym& fullCovariance);

    /**
     * Build the full covariance matrix of the residuals of the measurements out of the full covariance matrix
     * of the track states. The returned matrix has the dimensionality of the number of measurements * their
     * dimensionality and relates the residuals of each measurements to each other residuals of an (other) measurement.
     *
     * This can easily be done using the full covariance matrix of the states, which is calculated using the
     * buildFullCovarianceMatrix method. The ful covariance matrix of the residuals and their inverse is returned.
     *
     * The fullResidualCovariance is eq. (17) in 0810.2241.
     */
    static bool buildFullResidualCovarianceMatrix(const RecoTrack& recoTrack,
                                                  const std::vector<int>& vDimMeas,
                                                  const TMatrixDSym& fullCovariance,
                                                  TMatrixDSym& fullResidualCovariance,
                                                  TMatrixDSym& inverseFullMeasurementCovariance);

    /**
     * Calculate the weighted residuals (weighted with the DAF weight) of each hit and the
     * time derivative of the residuals by calling the timeDerivativesMeasurementsOnPlane method of the CDCRecoHits.
     *
     * The residuals as well as the residualTimeDerivative are returned by reference to a vector as large as there
     * are measurements in the reco track (exactly: number of measurements * dimensionality of each measurement).
     */
    static void buildResidualsAndTimeDerivative(const RecoTrack& recoTrack,
                                                const std::vector<int>& vDimMeas,
                                                TVectorD& residuals,
                                                TVectorD& residualTimeDerivative);


    /// Small helper function to extract the reduced chi^2 (chi^2/ndf) for python analyses. Returns NaN if ndf is 0.
    static double extractReducedChi2(const RecoTrack& recoTrack)
    {
      const double chi2 = recoTrack.getTrackFitStatus()->getChi2();
      const double ndf = recoTrack.getTrackFitStatus()->getNdf();

      if (ndf == 0) {
        return std::nan("");
      } else {
        return chi2 / ndf;
      }
    }
  };
}
