/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  addParam("vtxHitsStoreArrayName", m_param_vtxHitsStoreArrayName, "StoreArray name of the input VTX hits.",
           m_param_vtxHitsStoreArrayName);
  addParam("pxdHitsStoreArrayName", m_param_pxdHitsStoreArrayName, "StoreArray name of the input PXD hits.",
           m_param_pxdHitsStoreArrayName);
  addParam("svdHitsStoreArrayName", m_param_svdHitsStoreArrayName, "StoreArray name of the input SVD hits.",
           m_param_svdHitsStoreArrayName);
  addParam("cdcHitsStoreArrayName", m_param_cdcHitsStoreArrayName, "StoreArray name of the input CDC hits.",
           m_param_cdcHitsStoreArrayName);
  addParam("bklmHitsStoreArrayName", m_param_bklmHitsStoreArrayName, "StoreArray name of the input BKLM hits.",
           m_param_bklmHitsStoreArrayName);
  addParam("eklmHitsStoreArrayName", m_param_eklmHitsStoreArrayName, "StoreArray name of the input EKLM hits.",
           m_param_eklmHitsStoreArrayName);
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

  RecoTrack::registerRequiredRelations(recoTracks,
                                       m_param_pxdHitsStoreArrayName,
                                       m_param_svdHitsStoreArrayName,
                                       m_param_vtxHitsStoreArrayName,
                                       m_param_cdcHitsStoreArrayName,
                                       m_param_bklmHitsStoreArrayName,
                                       m_param_eklmHitsStoreArrayName,
                                       m_param_recoHitInformationStoreArrayName);
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
                                                             m_param_cdcHitsStoreArrayName, m_param_svdHitsStoreArrayName, m_param_pxdHitsStoreArrayName, m_param_vtxHitsStoreArrayName,
                                                             m_param_recoHitInformationStoreArrayName,
                                                             "", "", m_param_recreateSortingParameters);


    newRecoTrack->addRelationTo(&trackCandidate);

    // Add also the MC information
    const int mcParticleID = trackCandidate.getMcTrackId();
    if (mcParticleID >= 0 and mcParticles.isOptional() and mcParticles.getEntries() > 0) {
      MCParticle* relatedMCParticle = mcParticles[mcParticleID];
      if (relatedMCParticle) {
        newRecoTrack->addRelationTo(relatedMCParticle);
      } else {
        B2WARNING("Related MCParticle is invalid. Can not make a relation.");
      }
    }
  }
}
