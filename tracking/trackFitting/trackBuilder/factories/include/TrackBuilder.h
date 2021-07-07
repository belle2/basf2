/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>
#include <TVector3.h>

namespace Belle2 {

  class RecoTrack;

  /** TrackBuilder class to create the Track/TrackFitResult mdst output from the RecoTrack.
   *
   * To use this class, iterate over all Reco tracks and call trackBuilder.storeTrackFromRecoTrack.
   * All fitted hypotheses will be stored into one Track with the TrackFitResult array indices stored in the Track.
   * for (auto& recoTrack : recoTracks) {
   *   trackBuilder.storeTrackFromRecoTrack(recoTrack);
   * }
   *
   * If you want to make sure that all hypotheses are fitted and stored, call the fitter again.
   * If the tracks are fitted already, this produces no overhead.
   * TrackFitter trackFitter;
   * TrackBuilder trackBuilder;
   * for (auto& recoTrack : recoTracks) {
   *   for (const auto& pdg : m_pdgCodes) {
   *      trackFitter.fit(recoTrack, Const::ParticleType(abs(pdg)));
   *   }
   *   trackBuilder.storeTrackFromRecoTrack(recoTrack);
   * }
   */
  class TrackBuilder {
  public:
    /** Constructor of the class.
     *
     * @param trackColName Name of the store array for tracks (output).
     * @param trackFitResultColName Name of the store array for track fit results (output).
     * @param mcParticleColName Name of the store array for MC particles (input, optional).
     *                          If given, the tracks are matched to MCParticles.
     * @param beamSpot Origin.
     * @param beamAxis Positive z-direction.
     */
    TrackBuilder(
      const std::string& trackColName,
      const std::string& trackFitResultColName,
      const std::string& mcParticleColName,
      const TVector3& beamSpot = TVector3(0., 0., 0.),
      const TVector3& beamAxis = TVector3(0., 0., 1.)
    ) :
      m_trackColName(trackColName),
      m_trackFitResultColName(trackFitResultColName),
      m_mcParticleColName(mcParticleColName),
      m_beamSpot(beamSpot),
      m_beamAxis(beamAxis)
    {};

    /** Stores a Belle2 Track from a Reco Track.
     *
     * Every fitted hypothesis will be extrapolated to the perigee and stored as a TrackFitResult when the fit and the
     * extrapolation were successful. We will only create a Track mdst object, when we are sure that we have at least one valid
     * hypothesis available. If we do not have this, we discard the track no matter what.
     *
     * The StoreArrayIndex is stored in the Belle2 Track, no relation is set.
     *
     * @param recoTrack: The reco track the fit results are stored for
     * @param useClosestHitToIP: Flag to turn on special handling which measurement to choose;
     *        especially useful for Cosmics
     * @param useBFieldAtHit: Flag to calculate the BField at the used hit (closest to IP or first one), instead of the
     *        one at the POCA. This is also useful for cosmics only.
     * @return
     */
    bool storeTrackFromRecoTrack(RecoTrack& recoTrack,
                                 const bool useClosestHitToIP = false, const bool useBFieldAtHit = false);

    /// Get the HitPattern in the VXD.
    static uint32_t getHitPatternVXDInitializer(const RecoTrack& recoTrack);
    /// Get the HitPattern in the CDC.
    static uint64_t getHitPatternCDCInitializer(const RecoTrack& recoTrack);

  private:
    /// TrackColName (output).
    std::string m_trackColName;
    /// TrackFitResultColName (output).
    std::string m_trackFitResultColName;
    /// MCParticleColName (input, optional).
    std::string m_mcParticleColName;
    ///  Extrapolation target, origin.
    TVector3 m_beamSpot;
    ///  Extrapolation target, positive z direction.
    TVector3 m_beamAxis;
  };

}
