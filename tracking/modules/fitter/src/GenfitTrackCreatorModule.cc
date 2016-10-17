/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/fitter/GenfitTrackCreatorModule.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/RelationArray.h>

#include <genfit/Track.h>

using namespace std;
using namespace Belle2;

REG_MODULE(GenfitTrackCreator)

GenfitTrackCreatorModule::GenfitTrackCreatorModule() :
  Module()
{
  setDescription("Module turning reco tracks to genfit tracks (will be unneeded once we use reco tracks everywhere).");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("genfitTracksStoreArrayName", m_param_genfitTrackStoreArrayName, "StoreArray name of the output tracks.",
           m_param_genfitTrackStoreArrayName);
  addParam("genfitTrackCandsStoreArrayName", m_param_genfitTrackCandsStoreArrayName,
           "StoreArray name of the related track candidates.",
           m_param_genfitTrackCandsStoreArrayName);
  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the input reco tracks.",
           m_param_recoTracksStoreArrayName);
}

void GenfitTrackCreatorModule::initialize()
{
  B2WARNING("This module is depricated as it uses genfit::Track(Cand)s instead of RecoTracks. It will be removed in the future. If you need information on the transition, please contact Nils Braun (nils.braun@kit.edu).");
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<genfit::Track> genfitTracks(m_param_genfitTrackStoreArrayName);
  genfitTracks.registerInDataStore();

  genfitTracks.registerRelationTo(recoTracks);

  StoreArray<genfit::TrackCand> genfitTrackCands(m_param_genfitTrackCandsStoreArrayName);
  genfitTrackCands.registerRelationTo(genfitTracks);
}

void GenfitTrackCreatorModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  StoreArray<genfit::Track> genfitTracks(m_param_genfitTrackStoreArrayName);

  StoreArray<genfit::TrackCand> genfitTrackCands(m_param_genfitTrackCandsStoreArrayName);

  // ugly...
  RelationArray relationsFromTrackCandsToTracks(genfitTrackCands, genfitTracks);
  unsigned int trackCounter = 0;

  for (RecoTrack& recoTrack : recoTracks) {
    if (recoTrack.wasFitSuccessful()) {
      genfitTracks.appendNew(RecoTrackGenfitAccess::getGenfitTrack(recoTrack));
      genfit::TrackCand* relatedTrackCand = recoTrack.getRelated<genfit::TrackCand>(
                                              m_param_genfitTrackCandsStoreArrayName);

      // Search for TrackCandidate in StoreArray. This is really really bad, but gnfit does not have a compatible StoreArray-interface.
      int relatedTrackCandidateCounter = -1;
      int trackCandidateCounter = 0;

      for (const genfit::TrackCand& trackCandidate : genfitTrackCands) {
        if (&trackCandidate == relatedTrackCand) {
          relatedTrackCandidateCounter = trackCandidateCounter;
          break;
        }
        trackCandidateCounter++;
      }

      if (relatedTrackCandidateCounter != -1) {
        relationsFromTrackCandsToTracks.add(relatedTrackCandidateCounter, trackCounter);
      } else {
        B2WARNING("Related TrackCandidate is invalid. Do not add relation.");
      }
      trackCounter++;
    }
  }
}
