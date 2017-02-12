/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/fitter/FittedTracksStorerModule.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>

using namespace std;
using namespace Belle2;

REG_MODULE(FittedTracksStorer)

FittedTracksStorerModule::FittedTracksStorerModule() :
  Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("inputRecoTracksStoreArrayName", m_param_inputRecoTracksStoreArrayName, "StoreArray name of the input reco tracks.",
           m_param_inputRecoTracksStoreArrayName);
  addParam("outputRecoTracksStoreArrayName", m_param_outputRecoTracksStoreArrayName, "StoreArray name of the output reco tracks.",
           m_param_outputRecoTracksStoreArrayName);
}

void FittedTracksStorerModule::initialize()
{
  StoreArray<RecoTrack> inputRecoTracks(m_param_inputRecoTracksStoreArrayName);
  inputRecoTracks.isRequired();

  StoreArray<RecoTrack> outputRecoTracks(m_param_outputRecoTracksStoreArrayName);
  outputRecoTracks.registerInDataStore();

  RecoTrack::registerRequiredRelations(outputRecoTracks);

  StoreArray<MCParticle> mcParticles;
  if (mcParticles.isOptional()) {
    outputRecoTracks.registerRelationTo(mcParticles);
  }

  StoreArray<Track> tracks;
  if (tracks.isOptional()) {
    tracks.registerRelationTo(outputRecoTracks);
  }
}


void FittedTracksStorerModule::event()
{
  StoreArray<RecoTrack> inputRecoTracks(m_param_inputRecoTracksStoreArrayName);
  StoreArray<RecoTrack> outputRecoTracks(m_param_outputRecoTracksStoreArrayName);

  StoreArray<MCParticle> mcParticles;
  StoreArray<Track> tracks;

  for (RecoTrack& recoTrack : inputRecoTracks) {
    if (recoTrack.wasFitSuccessful()) {
      auto newRecoTrack = outputRecoTracks.appendNew(recoTrack.getPositionSeed(),
                                                     recoTrack.getMomentumSeed(),
                                                     recoTrack.getChargeSeed());
      newRecoTrack->addHitsFromRecoTrack(&recoTrack);

      // Add also relations
      auto relatedTrack = recoTrack.getRelated<Track>();
      if (relatedTrack) {
        relatedTrack->addRelationTo(newRecoTrack);
      }

      auto relatedParticle = recoTrack.getRelated<MCParticle>();
      if (relatedParticle) {
        newRecoTrack->addRelationTo(relatedParticle);
      }
    }
  }
}
