/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Kink.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/Track.h>

#include <Math/Vector3D.h>

#include <utility>

namespace genfit {
  class MeasuredStateOnPlane;
  class GFRaveVertex;
  class Track;
}

namespace Belle2 {

  /**
   * kinkFitter class to create Kink mdst's from reconstructed tracks.
   * To use this class, give the kinkFitter a mother and a daughter charged track and call the fitAndStore function.
   *
   * kinkFitter.fitAndStore(B2TrackMother, B2TrackDaughter, filterFlag);
   */
  class kinkFitter {

  public:
    /// Constructor for the kinkFitter.
    kinkFitter(const std::string& trackFitResultsName = "", const std::string& kinksName = "",
               const std::string& recoTracksName = "",
               const std::string& copiedRecoTracksName = "CopiedRecoTracks");

    /// Initialize the cuts which will be applied during the fit and store process.
    void initializeCuts(double vertexDistanceCut,
                        double vertexChi2Cut);

    /// set kink fitter mode.
    /// switch the mode of fitAndStore function.
    ///   0: store kink at the first vertex fit, regardless hit reassignment and flipping
    ///   1: hit reassignment between mother and daughter tracks
    ///   2: track flipping
    ///   3: option 1 and 2
    void setFitterMode(int fitterMode);

    /// Fit kink with cardinal hypothesis and store it if the fit was successful.
    /// If the corresponding flag is set, try to reassign hits between mother and daughter tracks.
    /// If the corresponding flag is set, try to flip and refit daughter track.
    /// filterFlag is used to distinguish filters with which pair was selected.
    bool fitAndStore(const Track* trackMother, const Track* trackDaughter, short filterFlag);

  private:

    /**
     * Combine daughter and mother tracks in one and fit.
     * @param trackMother mother Track
     * @param trackDaughter daughter Track
     * @return The filled bits of the fit result.
     * first bit: combined pValue > mother pValue;
     * second bit: combined pValue > daughter pValue
     * third bit: combined NDF > daughter NDF;
     * fourth bit: combined pValue > 10^-7;
     * 18: combined NDF < mother NDF;
     * 19: fit failed;
     */
    int combineTracksAndFit(const Track* trackMother, const Track* trackDaughter);

    /**
     * Find hit position closest to the vertex.
     * @param recoTrack RecoTrack
     * @param vertexPos vertex
     * @param direction direction (positive for daughter track, negative for mother track)
     * @return
     */
    int findHitPositionForReassignment(RecoTrack* recoTrack,
                                       ROOT::Math::XYZVector& vertexPos,
                                       int direction);

    /**
     * Copy RecoTrack to a separate StoreArray and reassign CDC hits according to delta
     * @param motherRecoTrack mother RecoTrack
     * @param daughterRecoTrack daughter RecoTrack
     * @param motherFlag true if creating mother, false if creating daughter
     * @param delta the position of the threshold to reassign the hits, starting from the end
     * positive if mother hits are reassigned to daughter, negative if vice-versa
     * @return a pointer to created RecoTrack
     */
    RecoTrack* copyRecoTrackAndReassignCDCHits(RecoTrack* motherRecoTrack, RecoTrack* daughterRecoTrack,
                                               bool motherFlag, int delta);

    /**
     * Try to fit new RecoTracks after hit reassignment.
     * @param recoTrackMotherRefit mother track after reassignment
     * @param recoTrackDaughterRefit daughter track after reassignment
     * @param recoTrackMother initial mother track
     * @param recoTrackDaughter initial daughter track
     * @return true if the fit was successful and the result for two tracks improved
     */
    bool refitRecoTrackAfterReassign(RecoTrack* recoTrackMotherRefit, RecoTrack* recoTrackDaughterRefit,
                                     RecoTrack* recoTrackMother, RecoTrack* recoTrackDaughter);

    /**
     * Flip and refit the daughter track.
     * @param recoTrack recoTrack to flip and refit
     * @param momentumSeed momentum seed (with the initial sign)
     * @param positionSeed position seed
     * @param timeSeed time seed
     * @return
     */
    RecoTrack* copyRecoTrackForFlipAndRefit(RecoTrack* recoTrack,
                                            ROOT::Math::XYZVector& momentumSeed,
                                            ROOT::Math::XYZVector& positionSeed,
                                            double& timeSeed);

    /**
     * Refit the daughter track blocking hits if required.
     * @param recoTrack recoTrack to refit
     * @param momentumSeed momentum seed
     * @param positionSeed position seed
     * @param timeSeed time seed
     * @param block block the hits in the first stereo layer and all before
     * @param useAnotherFitter use ordinary KalmanFilter
     * @return
     */
    RecoTrack* copyRecoTrackForRefit(RecoTrack* recoTrack,
                                     ROOT::Math::XYZVector& momentumSeed,
                                     ROOT::Math::XYZVector positionSeed,
                                     double& timeSeed,
                                     bool block, bool useAnotherFitter);

    /**
     * check if the refit of filter 6 daughter tracks improves the distance between mother and daughter
     * @param recoTrackDaughterRefit refitted daughter recoTrack
     * @param motherPosLast position of the mother state at last hit
     * @return
     */
    bool isRefitImproveFilter6(RecoTrack* recoTrackDaughterRefit, TVector3& motherPosLast);

    /**
     * Fit kink vertex using RecoTrack's as inputs.
     * Return true (false) if the vertex fit has done well (failed).
     * If the vertex is inside one of the RecoTracks, bits in reassignHitStatus are set.
     * @param recoTrackMother RecoTrack of mother
     * @param recoTrackDaughter RecoTrack of daughter
     * @param hasInnerHitStatus store a result of this function. if the plus(minus) track has hits inside the Kink vertex position, 0x1(0x2) bit is set.
     * @param vertexPos store a result of this function. The fitted vertex position is stored.
     * @param distance store a distance between tracks at the decay vertex
     * @param vertexPosSeed a seed of the vertex position
     * @return
     */
    bool vertexFitWithRecoTracks(RecoTrack* recoTrackMother, RecoTrack* recoTrackDaughter,
                                 unsigned int& reassignHitStatus,
                                 ROOT::Math::XYZVector& vertexPos, double& distance,
                                 ROOT::Math::XYZVector vertexPosSeed);

    /**
     * Extrapolate the fit results to the perigee to the vertex.
     * If the vertex is inside one of the tracks, bits in reassignHitStatus are set.
     * @param stMother mother MeasuredStateOnPlane
     * @param stDaughter daughter MeasuredStateOnPlane
     * @param vertexPosition vertex tracks to be extrapolated to
     * @param reassignHitStatus bits to be set in case of vertex being inside one of the tracks
     * @return
     */
    bool extrapolateToVertex(genfit::MeasuredStateOnPlane& stMother, genfit::MeasuredStateOnPlane& stDaughter,
                             const ROOT::Math::XYZVector& vertexPosition, unsigned int& reassignHitStatus);

    /**
     * Build TrackFitResult of Kink Track.
     * @param recoTrack input RecoTrack
     * @param msop MeasuredStateOnPlane extrapolated to the vertex (mother and daughter) or IP (mother)
     * @param Bz z component of the magnetic field at vertex or IP
     * @param trackHypothesis track hypothesis
     * @return
     */
    TrackFitResult* buildTrackFitResult(RecoTrack* recoTrack, const genfit::MeasuredStateOnPlane& msop,
                                        const double Bz, const Const::ParticleType trackHypothesis);


  private:
    // variables used for input
    std::string m_recoTracksName;   ///< RecoTrackColName (input).
    StoreArray <RecoTrack> m_recoTracks; ///< RecoTrack (input)

    // variables used for output
    StoreArray <TrackFitResult> m_trackFitResults;  ///< TrackFitResult (output).
    StoreArray <Kink> m_kinks;  ///< Kink (output).

    // variables used for output tmp (to be removed in the final version) TODO: remove
    StoreArray <RecoTrack> m_motherKinkRecoTracks;
    StoreArray <RecoTrack> m_daughterKinkRecoTracks;

    // cut variables
    double m_vertexDistanceCut;  ///< cut on the distance at the found vertex.
    double m_vertexChi2Cut;  ///< Chi2 cut.

    // fitter working mode variables
    int m_kinkFitterMode;  ///< fitter mode written in bits
    ///< first: reassign hits; second: flip and refit filter 2 tracks; third: combine tracks and fit them to find clones
    bool m_kinkFitterModeHitsReassignment; ///< fitter mode first bit
    bool m_kinkFitterModeFlipAndRefit; ///< fitter mode second bit
    bool m_kinkFitterModeCombineAndFit; ///< fitter mode third bit

    // helper variables
    StoreArray <RecoTrack> m_copiedRecoTracks; ///< RecoTrack used to refit tracks
    genfit::MeasuredStateOnPlane m_stMotherBuffer; ///< buffer for the MeasuredStateOnPlane of mother obtained in the vertex fit
    genfit::MeasuredStateOnPlane m_stDaughterBuffer; ///< buffer for the MeasuredStateOnPlane of daughter obtained in the vertex fit
    RecoTrack* m_motherKinkRecoTrackCache; ///< cash for the RecoTrack of mother used to find the best vertex
    RecoTrack* m_daughterKinkRecoTrackCache; ///< cash for the RecoTrack of daughter used to find the best vertex
  };

}

