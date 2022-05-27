/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/fitter/FittedTracksStorerModule.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>

using namespace std;
using namespace Belle2;

REG_MODULE(FittedTracksStorer);

FittedTracksStorerModule::FittedTracksStorerModule() :
  Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  setDescription("A module to copy only the fitted reco tracks to the output store array.");

  addParam("inputRecoTracksStoreArrayName", m_param_inputRecoTracksStoreArrayName, "StoreArray name of the input reco tracks.",
           m_param_inputRecoTracksStoreArrayName);
  addParam("outputRecoTracksStoreArrayName", m_param_outputRecoTracksStoreArrayName, "StoreArray name of the output reco tracks.",
           m_param_outputRecoTracksStoreArrayName);
  addParam("minimalWeight", m_param_minimalWeight, "Minimal weight for copying the hits.", m_param_minimalWeight);
}

void FittedTracksStorerModule::initialize()
{
  StoreArray<RecoTrack> inputRecoTracks(m_param_inputRecoTracksStoreArrayName);
  inputRecoTracks.isRequired();

  StoreArray<RecoTrack> outputRecoTracks(m_param_outputRecoTracksStoreArrayName);
  outputRecoTracks.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

  RecoTrack::registerRequiredRelations(outputRecoTracks);

  inputRecoTracks.registerRelationTo(outputRecoTracks);

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

  for (RecoTrack& recoTrack : inputRecoTracks) {
    if (recoTrack.wasFitSuccessful()) {
      auto newRecoTrack = recoTrack.copyToStoreArray(outputRecoTracks);
      newRecoTrack->addHitsFromRecoTrack(&recoTrack, 0, false, m_param_minimalWeight);

      // Add also relations
      auto relatedTrack = recoTrack.getRelated<Track>();
      if (relatedTrack) {
        relatedTrack->addRelationTo(newRecoTrack);
      }

      auto relatedParticle = recoTrack.getRelated<MCParticle>();
      if (relatedParticle) {
        newRecoTrack->addRelationTo(relatedParticle);
      }

      recoTrack.addRelationTo(newRecoTrack);
    }
  }
}
