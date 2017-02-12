/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Markus Prim                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>

#include <string>
#include <TVector3.h>

namespace Belle2 {

  class Track;
  class RecoTrack;
  class TrackFitResult;
  class HitPatternVXD;
  class HitPatternCDC;

  /*** TrackBuilder class to create the Track/TrackFitResult mdst output from the RecoTrack.
   *
   * To use this class, iterate over all Reco tracks and call trackBuilder.storeTrackFromRecoTrack. All fitted hypotheses will be stored into one Track with the TrackFitResult array indices stored in the Track.
   * for (auto& recoTrack : recoTracks) {
   *   trackBuilder.storeTrackFromRecoTrack(recoTrack);
   * }
   *
   * If you want to make sure that all hypotheses are fitted and stored, call the fitter again. If the tracks are fitted already, this produces no overhead.
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
    /** Constructor of the class. Requires the Store Arrayse and the extrapolation target (at Belle2 defined as the perigee in xy) for the TrackFitResults.
     *
     * @param trackColName TrackColName (output).
     * @param trackFitResultColName TrackFitResultColName (output).
     * @param mcParticleColName MCParticleColName (input, optional). If given, the tracks are matched to MCParticles.
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
     * extrapolation were successful. We will only create a Track mdst object, when we are sure that we have a valid
     * default (pion) hypothesis available. If we do not have this, we discard the track no matter what.
     *
     * The StoreArrayIndex is stored in the Belle2 Track, no relation is set.
     *
     * @param recoTrack:
     * @param useClosestHitInIP: Flag to turn on special handling which measurement "
    "to choose; especially useful for Cosmics
     * @return
     */
    bool storeTrackFromRecoTrack(RecoTrack& recoTrack, const Const::ParticleType& defaultHypothesis,
                                 const bool useClosestHitToIP = false);

  private:
    std::string m_trackColName;  ///< TrackColName (output).
    std::string m_trackFitResultColName;  ///< TrackFitResultColName (output).
    std::string m_mcParticleColName;  ///< MCParticleColName (input, optional).
    TVector3 m_beamSpot;  ///<  Extrapolation target, origin.
    TVector3 m_beamAxis;  ///<  Extrapolation target, positive z direction.

    uint32_t getHitPatternVXDInitializer(const RecoTrack& recoTrack) const;  ///< Get the HitPattern in the VXD.
    uint64_t getHitPatternCDCInitializer(const RecoTrack& recoTrack) const;  ///< Get the HitPattern in the CDC.
  };

}
