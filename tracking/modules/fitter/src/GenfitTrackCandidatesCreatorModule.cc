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

#include <mdst/dataobjects/MCParticle.h>

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
  addParam("mcGenfitTrackCandsStoreArrayName", m_param_mcGenfitTrackCandsStoreArrayName,
           "StoreArray name of the related track MC candidates (or empty if there are none).",
           m_param_mcGenfitTrackCandsStoreArrayName);
  addParam("mcRecoTracksStoreArrayName", m_param_mcRecoTracksStoreArrayName,
           "StoreArray name of the input reco MC tracks (or empty of there are none).",
           m_param_mcRecoTracksStoreArrayName);
  addParam("outputMCTrackCandidates", m_param_outputMCTrackCandidates,
           "Flag to turn on combined conversion of PR and MCTrackCands. Can only be used if the MCMatcher was used before.",
           m_param_outputMCTrackCandidates);
}

void GenfitTrackCandidatesCreatorModule::initialize()
{
  B2WARNING("This module is depricated as it uses genfit::Track(Cand)s instead of RecoTracks. It will be removed in the future. If you need information on the transition, please contact Nils Braun (nils.braun@kit.edu).");
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<genfit::TrackCand> genfitTrackCands(m_param_genfitTrackCandsStoreArrayName);
  genfitTrackCands.registerInDataStore();

  genfitTrackCands.registerRelationTo(recoTracks);

  StoreArray<MCParticle> mcParticles;
  if (mcParticles.isOptional() and recoTracks.optionalRelationTo(mcParticles)) {
    genfitTrackCands.registerRelationTo(mcParticles);
  }

  if (m_param_outputMCTrackCandidates) {
    StoreArray<RecoTrack> mcRecoTracks(m_param_mcRecoTracksStoreArrayName);
    mcRecoTracks.isRequired();

    StoreArray<genfit::TrackCand> mcGenfitTrackCands(m_param_mcGenfitTrackCandsStoreArrayName);
    mcGenfitTrackCands.registerInDataStore();

    mcGenfitTrackCands.registerRelationTo(mcRecoTracks);

    mcGenfitTrackCands.registerRelationTo(genfitTrackCands);
    genfitTrackCands.registerRelationTo(mcGenfitTrackCands);

    mcParticles.isRequired();
    mcGenfitTrackCands.registerRelationTo(mcParticles);
  }
}

void GenfitTrackCandidatesCreatorModule::outputPRAndMCTrackCands(const StoreArray<RecoTrack>& prRecoTracks,
    StoreArray<genfit::TrackCand>& prGenfitTrackCands,
    const StoreArray<MCParticle>& mcParticles, RelationArray& relationsFromPRTrackCandsToPRRecoTracks,
    RelationArray& relationsFromPRTrackCandsToMCParticle) const
{

  StoreArray<RecoTrack> mcRecoTracks(m_param_mcRecoTracksStoreArrayName);
  StoreArray<genfit::TrackCand> mcGenfitTrackCands(m_param_mcGenfitTrackCandsStoreArrayName);

  // ugly...
  RelationArray relationsFromMCTrackCandsToMCRecoTracks(mcGenfitTrackCands, mcRecoTracks);
  RelationArray relationsFromMCTrackCandsToMCParticle(mcGenfitTrackCands, mcParticles);

  for (const RecoTrack& prRecoTrack : prRecoTracks) {
    const int trackCounter = prRecoTrack.getArrayIndex();

    genfit::TrackCand* newGenfitTrackCand = prGenfitTrackCands.appendNew(prRecoTrack.createGenfitTrackCand());
    relationsFromPRTrackCandsToPRRecoTracks.add(trackCounter, trackCounter);

    // Read the matching status and encode it into the McTrackId of the genfit::TrackCand.
    const RecoTrack::MatchingStatus matchingStatus = prRecoTrack.getMatchingStatus();
    if (matchingStatus == RecoTrack::MatchingStatus::c_ghost) {
      newGenfitTrackCand->setMcTrackId(-999);
    } else if (matchingStatus == RecoTrack::MatchingStatus::c_background) {
      newGenfitTrackCand->setMcTrackId(-99);
    } else if (matchingStatus == RecoTrack::MatchingStatus::c_clone) {
      newGenfitTrackCand->setMcTrackId(-9);
    } else if (matchingStatus == RecoTrack::MatchingStatus::c_matched) {
      MCParticle* relatedMCParticle = prRecoTrack.getRelatedTo<MCParticle>();
      if (relatedMCParticle) {
        const int mcParticleArrayIndex = relatedMCParticle->getArrayIndex();
        relationsFromPRTrackCandsToMCParticle.add(trackCounter, mcParticleArrayIndex);
        newGenfitTrackCand->setMcTrackId(mcParticleArrayIndex);
      } else {
        B2WARNING("PRRecoTrack marked as 'matched', but no related MCParticle found. Something is going wrong here.");
      }
    }
  }

  for (const RecoTrack& mcRecoTrack : mcRecoTracks) {
    const int trackCounter = mcRecoTrack.getArrayIndex();

    genfit::TrackCand* newGenfitTrackCand = mcGenfitTrackCands.appendNew(mcRecoTrack.createGenfitTrackCand());
    relationsFromMCTrackCandsToMCRecoTracks.add(trackCounter, trackCounter);

    MCParticle* relatedMCParticle = mcRecoTrack.getRelatedTo<MCParticle>();
    if (relatedMCParticle) {
      const int mcParticleArrayIndex = relatedMCParticle->getArrayIndex();
      relationsFromMCTrackCandsToMCParticle.add(trackCounter, mcParticleArrayIndex);
      newGenfitTrackCand->setMcTrackId(mcParticleArrayIndex);
    } else {
      B2WARNING("MCRecoTrack without related MCParticle? Something is going wrong here.");
    }
  }

  // Add the relations between the MCTRackCands and the PRTrackCands if needed.
  RelationArray relationsFromMCTrackCandsToPRTrackCands(mcGenfitTrackCands, prGenfitTrackCands);
  RelationArray relationsFromPRTrackCandsToMCTrackCands(prGenfitTrackCands, mcGenfitTrackCands);

  for (const RecoTrack& prRecoTrack : prRecoTracks) {
    const auto& relatedMCRecoTrackWithWeight = prRecoTrack.getRelatedToWithWeight<RecoTrack>(m_param_mcRecoTracksStoreArrayName);
    RecoTrack* relatedMCRecoTrack = relatedMCRecoTrackWithWeight.first;
    const double weight = relatedMCRecoTrackWithWeight.second;
    if (relatedMCRecoTrack) {
      relationsFromPRTrackCandsToMCTrackCands.add(prRecoTrack.getArrayIndex(), relatedMCRecoTrack->getArrayIndex(), weight);
    }
  }

  for (const RecoTrack& mcRecoTrack : mcRecoTracks) {
    const auto& relatedPRRecoTrackWithWeight = mcRecoTrack.getRelatedToWithWeight<RecoTrack>(m_param_recoTracksStoreArrayName);
    RecoTrack* relatedPRRecoTrack = relatedPRRecoTrackWithWeight.first;
    const double weight = relatedPRRecoTrackWithWeight.second;
    if (relatedPRRecoTrack) {
      relationsFromMCTrackCandsToPRTrackCands.add(mcRecoTrack.getArrayIndex(), relatedPRRecoTrack->getArrayIndex(), weight);
    }
  }
}

void GenfitTrackCandidatesCreatorModule::outputPRTrackCands(const StoreArray<RecoTrack>& recoTracks,
                                                            StoreArray<genfit::TrackCand>& genfitTrackCands,
                                                            const StoreArray<MCParticle>&, RelationArray& relationsFromTrackCandsToRecoTracks,
                                                            RelationArray& relationsFromTrackCandsToMCParticle) const
{
  for (const RecoTrack& recoTrack : recoTracks) {
    const int trackCounter = recoTrack.getArrayIndex();

    genfit::TrackCand* newGenfitTrackCand = genfitTrackCands.appendNew(recoTrack.createGenfitTrackCand());
    relationsFromTrackCandsToRecoTracks.add(trackCounter, trackCounter);

    MCParticle* relatedMCParticle = recoTrack.getRelatedTo<MCParticle>();
    if (relatedMCParticle) {
      const int mcParticleArrayIndex = relatedMCParticle->getArrayIndex();
      relationsFromTrackCandsToMCParticle.add(trackCounter, mcParticleArrayIndex);
      newGenfitTrackCand->setMcTrackId(mcParticleArrayIndex);
    }
  }
}

void GenfitTrackCandidatesCreatorModule::event()
{
  StoreArray<MCParticle> mcParticles;

  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  StoreArray<genfit::TrackCand> genfitTrackCands(m_param_genfitTrackCandsStoreArrayName);

  // ugly...
  RelationArray relationsFromTrackCandsToRecoTracks(genfitTrackCands, recoTracks);
  RelationArray relationsFromTrackCandsToMCParticle(genfitTrackCands, mcParticles);

  if (m_param_outputMCTrackCandidates) {
    outputPRAndMCTrackCands(recoTracks, genfitTrackCands, mcParticles, relationsFromTrackCandsToRecoTracks,
                            relationsFromTrackCandsToMCParticle);
  } else {
    outputPRTrackCands(recoTracks, genfitTrackCands, mcParticles, relationsFromTrackCandsToRecoTracks,
                       relationsFromTrackCandsToMCParticle);
  }
}
