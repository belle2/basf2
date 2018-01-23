/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackCreator/TrackCreatorModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/trackBuilder/factories/TrackBuilder.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

using namespace Belle2;

REG_MODULE(TrackCreator)

TrackCreatorModule::TrackCreatorModule() :
  Module()
{
  setDescription(
    "Build Tracks with the TrackFitResults. Needs RecoTracks as input, creates Belle2::Tracks and Belle2::TrackFitResults as output.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // input
  addParam("recoTrackColName", m_recoTrackColName, "Name of collection holding the RecoTracks (input).",
           m_recoTrackColName);
  addParam("mcParticleColName", m_mcParticleColName, "Name of collection holding the MCParticles (input, optional).",
           m_mcParticleColName);
  // output
  addParam("trackColName", m_trackColName, "Name of collection holding the Tracks (output).", m_trackColName);
  addParam("trackFitResultColName", m_trackFitResultColName, "Name of collection holding the TrackFitResult (output).",
           m_trackFitResultColName);

  addParam("beamSpot", m_beamSpot,
           "BeamSpot (and BeamAxis) define the coordinate system in which the tracks will be extrapolated to the perigee.",
           m_beamSpot);
  addParam("beamAxis", m_beamAxis,
           "(BeamSpot and )BeamAxis define the coordinate system in which the tracks will be extrapolated to the perigee.",
           m_beamAxis);
  addParam("pdgCodes", m_pdgCodes,
           "PDG codes for which TrackFitResults will be created.",
           m_pdgCodes);

  addParam("useClosestHitToIP", m_useClosestHitToIP, "Flag to turn on special handling which measurement "
           "to choose; especially useful for Cosmics.", m_useClosestHitToIP);
  addParam("useBFieldAtHit", m_useBFieldAtHit, "Flag to calculate the BField at the used hit "
           "(closest to IP or first one), instead of the one at the POCA. Use this for cosmics to prevent problems, when cosmics reconstruction end up in the QCS magnet.",
           m_useBFieldAtHit);

}

void TrackCreatorModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_recoTrackColName);
  recoTracks.isRequired();

  StoreArray<MCParticle> mcParticles(m_mcParticleColName);
  const bool mcParticlesPresent = mcParticles.isOptional();

  StoreArray<Track> tracks(m_trackColName);
  const bool tracksRegistered = tracks.registerInDataStore();
  StoreArray<TrackFitResult> trackFitResults(m_trackFitResultColName);
  const bool trackFitResultsRegistered = trackFitResults.registerInDataStore();

  B2ASSERT(tracksRegistered and trackFitResultsRegistered, "Could not register output store arrays.");

  tracks.registerRelationTo(recoTracks);

  if (mcParticlesPresent) {
    tracks.registerRelationTo(mcParticles);
  }

  B2ASSERT("BeamSpot should have exactly 3 parameters", m_beamSpot.size() == 3);
  m_beamSpotAsTVector = TVector3(m_beamSpot[0], m_beamSpot[1], m_beamSpot[2]);

  B2ASSERT("BeamAxis should have exactly 3 parameters", m_beamAxis.size() == 3);
  m_beamAxisAsTVector = TVector3(m_beamAxis[0], m_beamAxis[1], m_beamAxis[2]);
}

void TrackCreatorModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_recoTrackColName);
  if (recoTracks.getEntries() == 0) {
    B2WARNING("RecoTrack StoreArray does not contain any RecoTracks.");
  }

  TrackFitter trackFitter;
  TrackBuilder trackBuilder(m_trackColName, m_trackFitResultColName, m_mcParticleColName,
                            m_beamSpotAsTVector, m_beamAxisAsTVector);
  for (auto& recoTrack : recoTracks) {
    for (const auto& pdg : m_pdgCodes) {
      // Does not refit in case the particle hypotheses demanded in this module have already been fitted before.
      // Otherwise fits them with the default fitter.
      B2DEBUG(200, "Trying to fit with PDG = " << pdg);
      trackFitter.fit(recoTrack, Const::ParticleType(abs(pdg)));
    }
    trackBuilder.storeTrackFromRecoTrack(recoTrack, m_useClosestHitToIP);
  }
}
