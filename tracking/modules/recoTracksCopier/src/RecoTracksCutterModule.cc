/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/recoTracksCopier/RecoTracksCutterModule.h>

using namespace Belle2;

REG_MODULE(RecoTracksCutter);

RecoTracksCutterModule::RecoTracksCutterModule() :
  Module()
{
  setDescription("Copies RecoTracks without their fit information.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("inputStoreArrayName", m_inputStoreArrayName,
           "Name of the input StoreArray");
  addParam("outputStoreArrayName", m_outputStoreArrayName,
           "Name of the output StoreArray");
  addParam("onlyFittedTracks", m_param_onlyFittedTracks, "Only copy fitted tracks", m_param_onlyFittedTracks);
  addParam("onlyGoodQITracks", m_param_onlyGoodQITracks, "Only copy good QI tracks", m_param_onlyGoodQITracks);
  addParam("minRequiredQuality", m_param_qiCutValue, "Minimum QI to copy tracks", m_param_qiCutValue);
}

void RecoTracksCutterModule::initialize()
{
  m_inputRecoTracks.isRequired(m_inputStoreArrayName);

  m_outputRecoTracks.registerInDataStore(m_outputStoreArrayName);
  RecoTrack::registerRequiredRelations(m_outputRecoTracks);

  m_outputRecoTracks.registerRelationTo(m_inputRecoTracks);

  if (m_tracks.optionalRelationTo(m_inputRecoTracks)) {
    m_tracks.registerRelationTo(m_outputRecoTracks);
  }
}

void RecoTracksCutterModule::event()
{
  for (const RecoTrack& recoTrack : m_inputRecoTracks) {
    if (m_param_onlyFittedTracks and not recoTrack.wasFitSuccessful()) {
      continue;
    }

    if (m_param_onlyGoodQITracks and recoTrack.getQualityIndicator() <  m_param_qiCutValue) {
      continue;
    }

    RecoTrack* newRecoTrack = recoTrack.copyToStoreArray(m_outputRecoTracks);
    newRecoTrack->addHitsFromRecoTrack(&recoTrack);
    newRecoTrack->addRelationTo(&recoTrack);

    for (Track& track : recoTrack.getRelationsWith<Track>()) {
      track.addRelationTo(newRecoTrack);
    }
  }
}

