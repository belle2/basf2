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
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("trackCandidatesStoreArrayName", m_param_trackCandidatesStoreArrayName, "StoreArray name of the input track candidates.",
           m_param_trackCandidatesStoreArrayName);
  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the output reco tracks.",
           m_param_recoTracksStoreArrayName);
  addParam("recoHitInformationStoreArrayName", m_param_recoHitInformationStoreArrayName,
           "StoreArray name of the output reco hit information.", m_param_recoHitInformationStoreArrayName);

  addParam("recreateSortingParameters", m_param_recreateSortingParameters,
           "Flag to recreate the sorting parameters of the hit out of the stored order.", m_param_recreateSortingParameters);

  addParam("cdcHitsStoreArrayName", m_param_cdcHitsStoreArrayName, "StoreArray name of the input cdc hits.",
           m_param_cdcHitsStoreArrayName);
  addParam("pxdHitsStoreArrayName", m_param_pxdHitsStoreArrayName, "StoreArray name of the input pxd hits.",
           m_param_pxdHitsStoreArrayName);
  addParam("svdHitsStoreArrayName", m_param_svdHitsStoreArrayName, "StoreArray name of the input svd hits.",
           m_param_svdHitsStoreArrayName);
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

  StoreArray<RecoHitInformation::UsedCDCHit> cdcHits(m_param_cdcHitsStoreArrayName);
  if (cdcHits.isOptional()) {
    cdcHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(cdcHits);
  }

  StoreArray<RecoHitInformation::UsedSVDHit> svdHits(m_param_svdHitsStoreArrayName);
  if (svdHits.isOptional()) {
    svdHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(svdHits);
  }

  StoreArray<RecoHitInformation::UsedPXDHit> pxdHits(m_param_pxdHitsStoreArrayName);
  if (pxdHits.isOptional()) {
    pxdHits.registerRelationTo(recoTracks);
    recoHitInformations.registerRelationTo(pxdHits);
  }

  recoTracks.registerRelationTo(recoHitInformations);
}

void RecoTrackCreatorModule::event()
{
  StoreArray<genfit::TrackCand> trackCandidates(m_param_trackCandidatesStoreArrayName);

  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  StoreArray<RecoHitInformation> recoHitInformations(m_param_recoHitInformationStoreArrayName);

  StoreArray<MCParticle> mcParticles;

  // ugly..

  for (const genfit::TrackCand& trackCandidate : trackCandidates) {

    if (trackCandidate.getNHits() < 3) {
      B2WARNING("Number of hits of track candidate is smaller than 3. Not creating track out of it.");
      continue;
    }

    RecoTrack* newRecoTrack = RecoTrack::createFromTrackCand(trackCandidate, m_param_recoTracksStoreArrayName,
                                                             m_param_cdcHitsStoreArrayName, m_param_svdHitsStoreArrayName, m_param_pxdHitsStoreArrayName,
                                                             m_param_recoHitInformationStoreArrayName,
                                                             "", "", m_param_recreateSortingParameters);


    newRecoTrack->addRelationTo(&trackCandidate);

    // Add also the MC information
    const int mcParticleID = trackCandidate.getMcTrackId();
    if (mcParticleID > 0 and mcParticles.isOptional()) {
      MCParticle* relatedMCParticle = mcParticles[mcParticleID];
      if (relatedMCParticle) {
        newRecoTrack->addRelationTo(relatedMCParticle);
      } else {
        B2WARNING("Related MCParticle is invalid. Can not make a relation.");
      }
    }
  }
}
