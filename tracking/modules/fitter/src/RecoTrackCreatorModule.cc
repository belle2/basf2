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
#include <mdst/dataobjects/MCParticle.h>

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
           "StoreArray name of the output reco hit information.", std::string("RecoHitInformations"));

  addParam("CDCHitsStoreArrayName", m_param_cdcHitsStoreArrayName, "StoreArray name of the input cdc hits.", std::string("CDCHits"));
  addParam("PXDHitsStoreArrayName", m_param_pxdHitsStoreArrayName, "StoreArray name of the input pxd hits.",
           std::string("PXDClusters"));
  addParam("SVDHitsStoreArrayName", m_param_svdHitsStoreArrayName, "StoreArray name of the input svd hits.",
           std::string("SVDClusters"));
}

void RecoTrackCreatorModule::initialize()
{
  // Read in genfit::TrackCands
  // Write our RecoTracks
  StoreArray<genfit::TrackCand> trackCandidates(m_param_trackCandidatesStoreArrayName);
  trackCandidates.isRequired();

  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.registerInDataStore();
  recoTracks.registerRelationTo(trackCandidates);
  trackCandidates.registerRelationTo(recoTracks);

  StoreArray<MCParticle> mcParticles;
  if (mcParticles.isOptional()) {
    recoTracks.registerRelationTo(mcParticles);
  }

  StoreArray<RecoHitInformation> recoHitInformations(m_param_recoHitInformationStoreArrayName);
  recoHitInformations.registerInDataStore();

  StoreArray<RecoTrack::UsedCDCHit> cdcHits(m_param_cdcHitsStoreArrayName);
  if (cdcHits.isOptional()) {
    cdcHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(cdcHits);
  }

  StoreArray<RecoTrack::UsedSVDHit> svdHits(m_param_svdHitsStoreArrayName);
  if (svdHits.isOptional()) {
    svdHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(svdHits);
  }

  StoreArray<RecoTrack::UsedPXDHit> pxdHits(m_param_pxdHitsStoreArrayName);
  if (pxdHits.isOptional()) {
    pxdHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(pxdHits);
  }

  recoTracks.registerRelationTo(recoHitInformations);
}

void RecoTrackCreatorModule::event()
{
  StoreArray <genfit::TrackCand> trackCandidates(m_param_trackCandidatesStoreArrayName);

  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.create();

  StoreArray<RecoHitInformation> recoHitInformations(m_param_recoHitInformationStoreArrayName);
  recoHitInformations.create();

  StoreArray<MCParticle> mcParticles;

  for (const genfit::TrackCand& trackCandidate : trackCandidates) {

    if (trackCandidate.getNHits() < 3) {
      B2WARNING("Number of hits of track candidate is smaller than 3. Not creating track out of it.");
      continue;
    }

    RecoTrack* newRecoTrack = RecoTrack::createFromTrackCand(&trackCandidate, m_param_recoTracksStoreArrayName,
                                                             m_param_cdcHitsStoreArrayName, m_param_svdHitsStoreArrayName, m_param_pxdHitsStoreArrayName,
                                                             m_param_recoHitInformationStoreArrayName);

    newRecoTrack->addRelationTo(&trackCandidate);
    DataStore::Instance().addRelationFromTo(&trackCandidate, newRecoTrack);

    // Add also the MC information
    const int mcParticleID = trackCandidate.getMcTrackId();
    if (mcParticleID > 0 and mcParticles.isOptional()) {
      MCParticle* relatedMCParticle = mcParticles[mcParticleID];
      newRecoTrack->addRelationTo(relatedMCParticle);
    }
  }
}
