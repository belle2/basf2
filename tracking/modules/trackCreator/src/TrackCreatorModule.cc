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

}

void TrackCreatorModule::initialize()
{
  B2WARNING("This module is still under development. "
            "It will not set relations which are currently used by modules in the post-tracking reconstruction and will never do so,"
            " because the desired workflow with the tracking dataobjects is via the RecoTracks. "
            "This does also not support multiple hypothesis yet.");

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
}

void TrackCreatorModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_recoTrackColName);
  if (recoTracks.getEntries() == 0) {
    B2WARNING("RecoTrack StoreArray does not contain any RecoTracks.");
  }

  TrackFitter trackFitter;
  TrackBuilder trackBuilder(m_trackColName, m_trackFitResultColName, m_mcParticleColName);
  for (auto& recoTrack : recoTracks) {
    // Require pion fit as a safety measure
    const bool pionFitWasSuccessful = trackFitter.fit(recoTrack, Const::ParticleType(abs(m_defaultPDGCode)));
    // Does not refit in case the particle hypotheses demanded in this module have already been fitted before.
    // Otherwise fits them with the default fitter.
    if (pionFitWasSuccessful) {
      for (const auto& pdg : m_additionalPDGCodes) {
        B2DEBUG(200, "Trying to fit with PDG = " << pdg);
        trackFitter.fit(recoTrack, Const::ParticleType(abs(pdg)));
      }
      trackBuilder.storeTrackFromRecoTrack(recoTrack, Const::ParticleType(abs(m_defaultPDGCode)), m_useClosestHitToIP);
    } else {
      B2DEBUG(200, "Pion fit failed - not creating a Track out of this RecoTrack.");
    }
  }
}