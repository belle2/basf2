/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/fitter/GenfitTrackCandidatesCreatorModule.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/RelationArray.h>

#include <genfit/Track.h>

using namespace std;
using namespace Belle2;

REG_MODULE(GenfitTrackCandidatesCreator)

GenfitTrackCandidatesCreatorModule::GenfitTrackCandidatesCreatorModule() :
  Module()
{
  setDescription("Module turning reco tracks to genfit tracks (will be unneeded once we use reco tracks everywhere).");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("genfitTrackCandsStoreArrayName", m_param_genfitTrackCandsStoreArrayName,
           "StoreArray name of the related track candidates.",
           m_param_genfitTrackCandsStoreArrayName);
  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the input reco tracks.",
           m_param_recoTracksStoreArrayName);
}

void GenfitTrackCandidatesCreatorModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<genfit::TrackCand> genfitTrackCands(m_param_genfitTrackCandsStoreArrayName);
  genfitTrackCands.registerInDataStore();

  genfitTrackCands.registerRelationTo(recoTracks);
}

void GenfitTrackCandidatesCreatorModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  StoreArray<genfit::TrackCand> genfitTrackCands(m_param_genfitTrackCandsStoreArrayName);
  genfitTrackCands.create();

  // ugly...
  RelationArray relationsFromTrackCandsToRecoTracks(genfitTrackCands, recoTracks);
  unsigned int trackCounter = 0;

  for (RecoTrack& recoTrack : recoTracks) {
    genfitTrackCands.appendNew(recoTrack.createGenfitTrackCand());
    relationsFromTrackCandsToRecoTracks.add(trackCounter, trackCounter);
    trackCounter++;
  }
}
