/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/recoTracksCopier/RecoTracksCopierModule.h>

using namespace Belle2;

REG_MODULE(RecoTracksCopier);

RecoTracksCopierModule::RecoTracksCopierModule() :
  Module()
{
  setDescription("Copies RecoTracks without their fit information.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("inputStoreArrayName", m_inputStoreArrayName,
           "Name of the input StoreArray");
  addParam("outputStoreArrayName", m_outputStoreArrayName,
           "Name of the output StoreArray");
  addParam("onlyFittedTracks", m_param_onlyFittedTracks, "Only copy fitted tracks", m_param_onlyFittedTracks);

}

void RecoTracksCopierModule::initialize()
{
  m_inputRecoTracks.isRequired(m_inputStoreArrayName);

  m_outputRecoTracks.registerInDataStore(m_outputStoreArrayName, DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_outputRecoTracks);

  m_outputRecoTracks.registerRelationTo(m_inputRecoTracks);
}

void RecoTracksCopierModule::event()
{
  for (const RecoTrack& recoTrack : m_inputRecoTracks) {
    if (m_param_onlyFittedTracks and not recoTrack.wasFitSuccessful()) {
      continue;
    }
    RecoTrack* newRecoTrack = recoTrack.copyToStoreArray(m_outputRecoTracks);
    newRecoTrack->addHitsFromRecoTrack(&recoTrack);
    newRecoTrack->addRelationTo(&recoTrack);
  }
}

