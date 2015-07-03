/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/fitter/RecoTrackCreatorModule.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

using namespace std;
using namespace Belle2;

REG_MODULE(RecoTrackCreator)

RecoTrackCreatorModule::RecoTrackCreatorModule() :
  Module()
{
  setDescription("Converts the given genfit::TrackCands in the StoreArray to RecoTracks for further use in the fitter.");

  addParam("TrackCandidatesStoreArrayName", m_param_trackCandidatesStoreArrayName, "StoreArray name of the input track candidates.",
           std::string("TrackCands"));
  addParam("RecoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the output reco tracks.",
           std::string("RecoTracks"));
  addParam("RecoHitInformationStoreArrayName", m_param_recoHitInformationStoreArrayName,
           "StoreArray name of the output reco hit information.", std::string("RecoHitInformation"));

  addParam("CDCHitsStoreArrayName", m_param_cdcHitsStoreArrayName, "StoreArray name of the input cdc hits.", std::string("CDCHits"));
  addParam("PXDHitsStoreArrayName", m_param_pxdHitsStoreArrayName, "StoreArray name of the input pxd hits.", std::string("PXDHits"));
  addParam("SVDHitsStoreArrayName", m_param_svdHitsStoreArrayName, "StoreArray name of the input svd hits.", std::string("SVDHits"));
}

void RecoTrackCreatorModule::initialize()
{
  // Read in genfit::TrackCands
  // Write our RecoTracks
  StoreArray<genfit::TrackCand> trackCandidates(m_param_trackCandidatesStoreArrayName);
  trackCandidates.isRequired();

  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.registerInDataStore();

  StoreArray<RecoHitInformation> recoHitInformations(m_param_recoHitInformationStoreArrayName);
  recoHitInformations.registerInDataStore();
}

void RecoTrackCreatorModule::event()
{
  StoreArray <genfit::TrackCand> trackCandidates(m_param_trackCandidatesStoreArrayName);

  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.create();

  StoreArray<RecoHitInformation> recoHitInformations(m_param_recoHitInformationStoreArrayName);
  recoHitInformations.create();

  for (const genfit::TrackCand& trackCandidate : trackCandidates) {
    RecoTrack::createFromTrackCand(&trackCandidate, m_param_recoTracksStoreArrayName,
                                   m_param_cdcHitsStoreArrayName, m_param_svdHitsStoreArrayName, m_param_pxdHitsStoreArrayName,
                                   m_param_recoHitInformationStoreArrayName);
  }
}
