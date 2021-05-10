#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/V0.h>
#include <tracking/dataobjects/V0ValidationVertex.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/Track.h>

#include <TVector3.h>

#include <utility>

namespace genfit {
  class MeasuredStateOnPlane;
  class GFRaveVertex;
  class Track;
}

namespace Belle2 {

  /** V0Fitter class to create V0 mdst's from reconstructed tracks.
   * To use this class, give the V0Fitter a positive and a negative charged track and call the fitAndStore function.
   *
   * v0Fitter.fitAndStore(B2TrackPositive, B2TrackNegative, V0HypothesisParticleType);
   */
  class V0Fitter {
    friend class V0FitterTest_GetTrackHypotheses_Test;
    friend class V0FitterTest_EnableValidation_Test;
    friend class V0FitterTest_InitializeCuts_Test;

  public:
    /// Constructor for the V0Fitter.
    V0Fitter(const std::string& trackFitResultsName = "", const std::string& v0sName = "",
             const std::string& v0ValidationVerticesName = "",
             const std::string& recoTracksName = "",
             const std::string& copiedRecoTracksName = "CopiedRecoTracks",
             bool enableValidation = false);

    /// Initialize the cuts which will be applied during the fit and store process.
    void initializeCuts(double beamPipeRadius,
                        double vertexChi2CutOutside,
                        std::tuple<double, double> invMassRangeKshort,
                        std::tuple<double, double> invMassRangeLambda,
                        std::tuple<double, double> invMassRangePhoton);

    /// set V0 fitter mode.
    /// switch the mode of fitAndStore function.
    ///   0: store V0 at the first vertex fit, regardless of inner hits
    ///   1: remove hits inside the V0 vertex position
    ///   2: mode 1 +  don't use SVD hits if there is only one available SVD hit-pair (default)
    void setFitterMode(int fitterMode);

    /// Fit V0 with given hypothesis and store if fit was successful.
    bool fitAndStore(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis);

    /// Get track hypotheses for a given v0 hypothesis.
    std::pair<Const::ParticleType, Const::ParticleType> getTrackHypotheses(const Const::ParticleType& v0Hypothesis) const;

  private:

    /** fit V0 vertex using RecoTrack's as inputs.
     * Return true (false) if the vertex fit has done well (failed).
     * If RecoTracks have hits inside the fitted V0 vertex position, bits in hasInnerHitStatus are set.
     * If there are no inside hits, store the V0 to the DataStore.
     * @param trackPlus Track of positively-charged daughter
     * @param trackMinus Track of negatively-charged daughter
     * @param recoTrackPlus RecoTrack of positively-charged daughter
     * @param recoTrackMinus RecoTrack of negatively-charged daughter
     * @param v0Hypothesis ParticleType used in vertex fitting
     * @param hasInnerHitStatus store a result of this function. if the plus(minus) track has hits inside the V0 vertex position, 0x1(0x2) bit is set.
     * @param vertexPos store a result of this function. the fitted vertex position is stored.
     * @param forceStore if true, store the fitted V0 to the DataStore even if there are some inside hits.
     * @return
     */
    bool vertexFitWithRecoTracks(const Track* trackPlus, const Track* trackMinus,
                                 RecoTrack* recoTrackPlus, RecoTrack* recoTrackMinus,
                                 const Const::ParticleType& v0Hypothesis,
                                 unsigned int& hasInnerHitStatus, TVector3& vertexPos,
                                 const bool forceStore);

    /** Create a copy of RecoTrack.
     * @param origRecoTrack original RecoTrack
     * @param trackPDG signed PDG used for the track fit hypothesis
     * @return copied RecoTrack stored in the m_copiedRecoTracks, nullptr if track fit fails (this should not happen)
     */
    RecoTrack* copyRecoTrack(RecoTrack* origRecoTrack, const int trackPDG);

    /** Remove inner hits from RecoTrack.
     * Hits are removed from the minus-end of the momentum direction.
     * For SVD hits, remove U- and V- hit pair at once.
     * Input RecoTrack is fitted in the funcion.
     * If track fit fails, return false.
     * @param origRecoTrack original RecoTrack
     * @param recoTrack input RecoTrack, updated in this function
     * @param trackPDG signed PDG used for the track fit hypothesis
     * @param nRemoveHits the number of removed hits. This can be incremented in the function if the outermost removed hit is an SVD U-hit.
     * @return
     */
    bool removeInnerHits(RecoTrack* origRecoTrack, RecoTrack* recoTrack,
                         const int trackPDG, unsigned int& nRemoveHits);

    /** Compare innremost hits of daughter pairs to check if they are the same (shared) or not.
     * For SVD hits, compare U- and V- hit pair.
     * @param recoTrackPlus, recoTrackMinus input RecoTrack pair
     * @return 0 for not the same (not shared), 1 for 1D-hit share (1-side SVD hit), 2 for 2D-hit (PXD hit or SVD hit pair) share, -1 for exception
     */
    int checkSharedInnermostCluster(const RecoTrack* recoTrackPlus, const RecoTrack* recoTrackMinus);

    /** Fit the V0 vertex.
     *
     * @param trackPlus
     * @param trackMinus
     * @param vertex Result of the fit is returned via reference.
     * @return
     */
    bool fitGFRaveVertex(genfit::Track& trackPlus, genfit::Track& trackMinus, genfit::GFRaveVertex& vertex);

    /// Extrapolate the fit results to the perigee to the vertex.
    bool extrapolateToVertex(genfit::MeasuredStateOnPlane& stPlus, genfit::MeasuredStateOnPlane& stMinus,
                             const TVector3& vertexPosition);

    /// Extrapolate the fit results to the perigee to the vertex.
    /// If the daughter tracks have hits inside the V0 vertex, bits in the hasInnerHiStatus variable are set.
    bool extrapolateToVertex(genfit::MeasuredStateOnPlane& stPlus, genfit::MeasuredStateOnPlane& stMinus,
                             const TVector3& vertexPosition, unsigned int& hasInnerHitStatus);

    /// Getter for magnetic field in z direction at the vertex position.
    double getBzAtVertex(const TVector3& vertexPosition);

    /// Build TrackFitResult of V0 Track and set relation to genfit Track.
    TrackFitResult* buildTrackFitResult(const genfit::Track& track, const RecoTrack* recoTrack,
                                        const genfit::MeasuredStateOnPlane& msop, const double Bz,
                                        const Const::ParticleType& trackHypothesis,
                                        const int sharedInnermostCluster);


  private:
    bool m_validation;  ///< Validation flag.
    std::string m_recoTracksName;   ///< RecoTrackColName (input).
    StoreArray<RecoTrack> m_recoTracks; ///< RecoTrack (input)
    StoreArray<TrackFitResult> m_trackFitResults;  ///< TrackFitResult (output).
    StoreArray<V0> m_v0s;  ///< V0 (output).
    StoreArray<V0ValidationVertex> m_validationV0s;  ///< V0ValidationVertex (output, optional).
    StoreArray<RecoTrack> m_copiedRecoTracks; ///< RecoTrack used to refit tracks (output)

    double m_beamPipeRadius;  ///< Radius where inside/outside beampipe is defined.
    double m_vertexChi2CutOutside;  ///< Chi2 cut outside beampipe.
    std::tuple<double, double> m_invMassRangeKshort; ///< invariant mass cut for Kshort.
    std::tuple<double, double> m_invMassRangeLambda; ///< invariant mass cut for Lambda.
    std::tuple<double, double> m_invMassRangePhoton; ///< invariant mass cut for Photon.
    int    m_v0FitterMode;  ///< 0: store V0 at the first vertex fit, regardless of inner hits, 1: remove hits inside the V0 vertex position, 2: mode 1 +  don't use SVD hits if there is only one available SVD hit-pair (default)
    bool   m_forcestore;///< true only if the V0Fitter mode is 1
    bool   m_useOnlyOneSVDHitPair;///< false only if the V0Fitter mode is 3
  };

}

