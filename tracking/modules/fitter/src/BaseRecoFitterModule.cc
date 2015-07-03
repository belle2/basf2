/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/AbsKalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/MaterialEffects.h>
#include <genfit/FieldManager.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticCDCGeometryTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/modules/fitter/BaseRecoFitterModule.h>

using namespace std;
using namespace Belle2;

BaseRecoFitterModule::BaseRecoFitterModule() :
  Module()
{
  setDescription("Fit the given reco tracks with the given fitter parameters.");

  addParam("RecoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the input and output reco tracks.",
           std::string("RecoTracks"));

  addParam("TracksStoreArrayName", m_param_tracksStoreArrayName, "StoreArray name of the output genfit::tracks.",
           std::string("GF2Tracks"));
  addParam("TracksCandsStoreArrayName", m_param_tracksCandsStoreArrayName, "StoreArray name of the input genfit::TrackCands.",
           std::string("TrackCands"));
}

void BaseRecoFitterModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<genfit::Track> tracks(m_param_tracksStoreArrayName);
  tracks.registerInDataStore();

  StoreArray<genfit::TrackCand> trackCands(m_param_tracksCandsStoreArrayName);
  trackCands.isRequired();

  trackCands.registerRelationTo(tracks);


  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  }

  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2FATAL("Magnetic field not set up. Please use SetupGenfitExtrapolationModule.");
  }

  // Create new Translators and give them to the CDCRecoHits.
  CDCRecoHit::setTranslators(new CDC::LinearGlobalADCCountTranslator(),
                             new CDC::RealisticCDCGeometryTranslator(true),
                             new CDC::RealisticTDCCountTranslator(true),
                             true);

}

void BaseRecoFitterModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  StoreArray<genfit::Track> tracks(m_param_tracksStoreArrayName);
  StoreArray<genfit::TrackCand> trackCandidates(m_param_tracksCandsStoreArrayName);
  tracks.create();

  RelationArray relationBetweenTrackCandidatesAndTracks(trackCandidates, tracks);
  relationBetweenTrackCandidatesAndTracks.create();

  const std::shared_ptr<genfit::AbsFitter>& fitter = createFitter();

  for (RecoTrack& recoTrack : recoTracks) {
    recoTrack.fit(fitter, 211);

    // TEMPORAL
    tracks.appendNew(recoTrack);

    const genfit::TrackCand* relatedTrackCandidate = recoTrack.getRelated<genfit::TrackCand>(m_param_tracksCandsStoreArrayName);
    int trackCandidatesIndex = 0;
    for (; trackCandidatesIndex < trackCandidates.getEntries(); trackCandidatesIndex++) {
      if (trackCandidates[trackCandidatesIndex] == relatedTrackCandidate)
        break;
    }
    unsigned int tracksIndex = tracks.getEntries() - 1;
    relationBetweenTrackCandidatesAndTracks.add(trackCandidatesIndex, tracksIndex);
  }
}
