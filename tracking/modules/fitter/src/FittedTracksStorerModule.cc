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
  m_inputRecoTracks.isRequired(m_param_inputRecoTracksStoreArrayName);
  m_outputRecoTracks.registerInDataStore(m_param_outputRecoTracksStoreArrayName, DataStore::c_ErrorIfAlreadyRegistered);

  RecoTrack::registerRequiredRelations(m_outputRecoTracks);
  m_inputRecoTracks.registerRelationTo(m_outputRecoTracks);

  if (m_MCParticles.isOptional()) {
    m_outputRecoTracks.registerRelationTo(m_MCParticles);
  }

  if (m_Tracks.isOptional()) {
    m_Tracks.registerRelationTo(m_outputRecoTracks);
  }
}


void FittedTracksStorerModule::event()
{
  for (RecoTrack& recoTrack : m_inputRecoTracks) {
    if (recoTrack.wasFitSuccessful()) {
      auto newRecoTrack = recoTrack.copyToStoreArray(m_outputRecoTracks);
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
