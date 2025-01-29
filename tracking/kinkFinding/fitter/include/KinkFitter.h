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
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <utility>

namespace Belle2 {

  /**
   * KinkFitter class to create Kink mdst's objects from reconstructed tracks.
   * To use this class, give the kinkFitter a mother and a daughter charged track and call the fitAndStore function.
   * In case of track splitting, the mother and daughter should be the same particle.
   *
   * kinkFitter.fitAndStore(B2TrackMother, B2TrackDaughter, filterFlag);
   */
  class KinkFitter {

  public:
    /**
     * Constructor for the KinkFitter.
     * @param trackFitResultsName Belle2::TrackFitResult StoreArray name.
     * @param kinksName Kink StoreArray name.
     * @param recoTracksName RecoTrack StoreArray name.
     * @param copiedRecoTracksName RecoTrack StoreArray name (used for track refitting).
     */
    KinkFitter(const std::string& trackFitResultsName = "", const std::string& kinksName = "",
               const std::string& recoTracksName = "",
               const std::string& copiedRecoTracksName = "RecoTracksKinkTmp");

    /**
     * Initialize the cuts which will be applied during the fit and store process.
     * @param vertexDistanceCut Cut on distance between tracks at the Kink vertex.
     * @param vertexChi2Cut Cut on Chi2 for the Kink vertex.
     * @param precutDistance Preselection cut on distance between ending points of two tracks used in KinkFinderModule.
     */
    void initializeCuts(const double vertexDistanceCut, const double vertexChi2Cut, const double precutDistance);

    /**
     * set kink fitter mode.
     * @param fitterMode from 0 to 15 in binary:
     * 1st bit: reassign hits (1 is On, 0 is Off)
     * 2nd bit: flip tracks with close end points (1 is On, 0 is Off)
     * 3rd bit: fit both tracks as one (1 is On, 0 is Off)
     * 4th bit: track splitting (1 is On, 0 is Off)
     */
    void setFitterMode(const unsigned char fitterMode);

    /**
     * Fit kink with cardinal hypothesis and store it if the fit was successful.
     * If the corresponding flag is set, try to reassign hits between mother and daughter tracks.
     * If the corresponding flag is set, try to flip and refit daughter track.
     * If the corresponding flag is set, try to combine mother and daughter track and fit the resulting track.
     * If the corresponding flag is set, try to split the track candidate selected by KinkFinderModule.
     * @param trackMother mother Track
     * @param trackDaughter daughter Track (in case of splitting the same as trackMother)
     * @param filterFlag filter with which track pair was selected
     * Filter 1: Distance between first point of the daughter and last point of the mother < m_precutDistance (majority).
     * Filter 2: Distance between last point of the daughter and last point of the mother < m_precutDistance
     * (wrong daughter sign).
     * Filter 3: Distance between the daughter Helix extrapolation to last point of the mother
     * and last point of the mother < m_precutDistance (lost layers for daughter, second largest contribution).
     * Filter 4: 2D distance between first point of the daughter and last point of the mother < m_precutDistance2D
     * (bad daughter resolution recovered by hit reassignment).
     * Filter 5: 2D distance between last point of the daughter and last point of the mother < m_precutDistance2D
     * (bad daughter resolution and wrong daughter sign, almost no events).
     * Filter 6: Distance between the daughter Helix extrapolation to last point of the mother
     * and last point of the mother < m_precutDistance2D
     * (lost layers for daughter combined with bad daughter resolution, can be recovered by refit).
     * Filter 7: Track to split selected among mother candidates.
     * Filter 8: Track to split selected among daughter candidates.
     * Filter 9: Track to split selected among tracks not passing mother/daughter criteria.
     * @return true if the track pair is stored as a Kink, false in other cases
     */
    bool fitAndStore(const Track* trackMother, const Track* trackDaughter, short filterFlag);

  private:

    /**
     * Create a RecoTrack in a separate StoreArray based on one to be split
     * @param splitRecoTrack RecoTrack to be split
     * @param motherFlag true if creating mother, false if creating daughter
     * @param delta the position of the threshold to split the hits, starting from the end
     * @return a pointer to created RecoTrack
     */
    RecoTrack* copyRecoTrackAndSplit(const RecoTrack* splitRecoTrack, const bool motherFlag, const unsigned int delta);

    /**
     * Split track into two based on |chi2/ndf - 1|. For the best split position search, use binary search.
     * @param recoTrackSplit RecoTrack to be split
     * @param recoTrackIndexMother index of the created mother RecoTrack in m_copiedRecoTracks
     * @param recoTrackIndexDaughter index of the created daughter RecoTrack in m_copiedRecoTracks
     * @return true if the splitting is successful, false otherwise
     */
    bool splitRecoTrack(const RecoTrack* recoTrackSplit, short& recoTrackIndexMother, short& recoTrackIndexDaughter);

    /**
     * Combine daughter and mother tracks in one and fit.
     * @param trackMother mother Track
     * @param trackDaughter daughter Track
     * @return The filled bits of the fit result.
     * first bit: combined pValue > mother pValue;
     * second bit: combined pValue > daughter pValue
     * third bit: combined NDF > daughter NDF;
     * fourth bit: combined pValue > 10^-7;
     * If the value > 15, the combination is assumed as failed with the following codes:
     * 18: combined NDF < mother NDF;
     * 19: fit failed;
     */
    unsigned int combineTracksAndFit(const Track* trackMother, const Track* trackDaughter);

    /**
     * Find hit position closest to the vertex.
     * @param recoTrack RecoTrack
     * @param vertexPos vertex
     * @param direction direction (+1 for daughter track, -1 for mother track). Should be +-1
     * @return for daughter track, returns negative index of the hit, closest to the vertex
     * for mother track, returns positive index of the hit, closest to the vertex, counting from the end of the track
     */
    int findHitPositionForReassignment(const RecoTrack* recoTrack,
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
                                               const bool motherFlag, const int delta);

    /**
     * Try to fit new RecoTracks after hit reassignment.
     * @param recoTrackMotherRefit mother track after reassignment
     * @param recoTrackDaughterRefit daughter track after reassignment
     * @param recoTrackMother initial mother track
     * @param recoTrackDaughter initial daughter track
     * @return true if the fit was successful and the result for two tracks improved
     */
    bool refitRecoTrackAfterReassign(RecoTrack* recoTrackMotherRefit, RecoTrack* recoTrackDaughterRefit,
                                     const RecoTrack* recoTrackMother, const RecoTrack* recoTrackDaughter);

    /**
     * Flip and refit the daughter track.
     * @param recoTrack recoTrack to flip and refit
     * @param momentumSeed momentum seed (with the initial sign)
     * @param positionSeed position seed
     * @param timeSeed time seed
     * @return pointer to a new copied flipped and refitted daughter RecoTrack
     */
    RecoTrack* copyRecoTrackForFlipAndRefit(const RecoTrack* recoTrack,
                                            const ROOT::Math::XYZVector& momentumSeed,
                                            const ROOT::Math::XYZVector& positionSeed,
                                            const double& timeSeed);

    /**
     * Refit the daughter track blocking hits if required.
     * @param recoTrack recoTrack to refit
     * @param momentumSeed momentum seed
     * @param positionSeed position seed
     * @param timeSeed time seed
     * @param blockInnerStereoHits block the hits in the first stereo layer and all before
     * @param useAnotherFitter use ordinary KalmanFilter
     * @return pointer to a new copied refitted daughter RecoTrack
     */
    RecoTrack* copyRecoTrackForRefit(const RecoTrack* recoTrack,
                                     const ROOT::Math::XYZVector& momentumSeed,
                                     const ROOT::Math::XYZVector& positionSeed,
                                     const double& timeSeed,
                                     const bool blockInnerStereoHits = false, const bool useAnotherFitter = false);

    /**
     * check if the refit of filter 6 daughter tracks improves the distance between mother and daughter
     * @param recoTrackDaughterRefit refitted daughter recoTrack
     * @param motherPosLast position of the mother state at last hit
     * @return true if the refit of filter 6 daughter tracks improves the distance between mother and daughter;
     * false otherwise
     */
    bool isRefitImproveFilter6(const RecoTrack* recoTrackDaughterRefit, const ROOT::Math::XYZVector& motherPosLast);

    /**
     * Fit kink vertex using RecoTrack's as inputs.
     * Return true (false) if the vertex fit has done well (failed).
     * If the vertex is inside one of the RecoTracks, bits in reassignHitStatus are set.
     * @param recoTrackMother RecoTrack of mother
     * @param recoTrackDaughter RecoTrack of daughter
     * @param reassignHitStatus store a result of this function. if the daughter(mother) track has hits inside
     * the Kink vertex position, 0x1(0x2) bit is set.
     * @param vertexPos store a result of this function. The fitted vertex position is stored.
     * @param distance store a distance between tracks at the decay vertex
     * @param vertexPosSeed a seed of the vertex position
     * @return true if the vertex fit succeed;
     * false if one of the tracks is not fitted (should not happen), kFit finishes with error,
     * chi^2 of the fit is too large, or extrapolation of one of the tracks to decay vertex fails
     */
    bool vertexFitWithRecoTracks(RecoTrack* recoTrackMother, RecoTrack* recoTrackDaughter,
                                 unsigned int& reassignHitStatus,
                                 ROOT::Math::XYZVector& vertexPos, double& distance,
                                 ROOT::Math::XYZVector vertexPosSeed = ROOT::Math::XYZVector(0, 0, 0));

    /**
     * Extrapolate the fit results to the perigee to the kink vertex.
     * If the vertex is inside one of the tracks, bits in reassignHitStatus are set.
     * @param stMother mother MeasuredStateOnPlane
     * @param stDaughter daughter MeasuredStateOnPlane
     * @param vertexPosition vertex tracks to be extrapolated to
     * @param reassignHitStatus bits to be set in case of vertex being inside one of the tracks
     * @return false if can not extrapolate one of the tracks to vertex;
     * true otherwise
     */
    bool extrapolateToVertex(genfit::MeasuredStateOnPlane& stMother, genfit::MeasuredStateOnPlane& stDaughter,
                             const ROOT::Math::XYZVector& vertexPosition, unsigned int& reassignHitStatus);

    /**
     * Build TrackFitResult of the Kink Track.
     * @param recoTrack input RecoTrack
     * @param msop MeasuredStateOnPlane extrapolated to the vertex (mother and daughter) or IP (mother)
     * @param Bz z component of the magnetic field at vertex or IP
     * @param trackHypothesis track hypothesis
     * @return pointer to created TrackFitResult of the Kink Track
     */
    TrackFitResult* buildTrackFitResult(RecoTrack* recoTrack, const genfit::MeasuredStateOnPlane& msop,
                                        const double Bz, const Const::ParticleType trackHypothesis);

    /**
     * Prepare the error matrix for the kFit.
     * @param fourMomentum four momentum of the track state to be used in kFit
     * @param covMatrix6 covariance matrix of the track state to be used in kFit
     * @param errMatrix7 error matrix of the track state to be prepared and used in kFit
     */
    void errMatrixForKFit(ROOT::Math::PxPyPzEVector& fourMomentum, TMatrixDSym& covMatrix6,
                          TMatrixDSym& errMatrix7);


  private:

    // variables used for input
    std::string m_recoTracksName;   ///< RecoTrackColName (input).
    StoreArray <RecoTrack> m_recoTracks; ///< RecoTrack (input)

    // variables used for output
    StoreArray <TrackFitResult> m_trackFitResults;  ///< TrackFitResult (output).
    StoreArray <Kink> m_kinks;  ///< Kink (output).

    // cut variables
    double m_vertexDistanceCut;  ///< cut on the distance at the found vertex.
    double m_vertexChi2Cut;  ///< Chi2 cut.
    double m_precutDistance;  ///< Preselection cut on distance between ending points of two tracks used in prefilter.
    ///< here it is needed in isRefitImproveFilter6 function

    // fitter working mode variables
    unsigned char m_kinkFitterMode;  ///< fitter mode from 0 to 15 written in bits:
    ///< 1st bit: reassign hits (1 is On, 0 is Off)
    ///< 2nd bit: flip tracks with close end points (1 is On, 0 is Off)
    ///< 3rd bit: fit both tracks as one (1 is On, 0 is Off)
    ///< 4th bit: combined track candidate splitting (1 is On, 0 is Off)
    bool m_kinkFitterModeHitsReassignment; ///< fitter mode 1st bit
    bool m_kinkFitterModeFlipAndRefit; ///< fitter mode 2nd bit
    bool m_kinkFitterModeCombineAndFit; ///< fitter mode 3rd bit
    bool m_kinkFitterModeSplitTrack; ///< fitter mode 4th bit

    // helper variables
    StoreArray <RecoTrack> m_copiedRecoTracks; ///< RecoTrack used to refit tracks
    genfit::MeasuredStateOnPlane m_stMotherBuffer; ///< buffer for the MeasuredStateOnPlane of mother obtained in the vertex fit
    genfit::MeasuredStateOnPlane m_stDaughterBuffer; ///< buffer for the MeasuredStateOnPlane of daughter obtained in the vertex fit
    RecoTrack* m_motherKinkRecoTrackCache; ///< cache for the RecoTrack of mother used to find the best vertex
    RecoTrack* m_daughterKinkRecoTrackCache; ///< cache for the RecoTrack of daughter used to find the best vertex
  };

}
