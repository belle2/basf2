/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Simon Kurz                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/relatedTracksCombiner/RecoTrackStoreArrayCombiner.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

using namespace Belle2;

REG_MODULE(RecoTrackStoreArrayCombiner);

RecoTrackStoreArrayCombinerModule::RecoTrackStoreArrayCombinerModule() :
  Module()
{
  setDescription("Combine two collections of tracks without additional checks.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("Temp1RecoTracksStoreArrayName", m_temp1RecoTracksStoreArrayName , "Name of the first input StoreArray.",
           m_temp1RecoTracksStoreArrayName);
  addParam("Temp2RecoTracksStoreArrayName", m_temp2RecoTracksStoreArrayName , "Name of the second input StoreArray.",
           m_temp2RecoTracksStoreArrayName);
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the output StoreArray.", m_recoTracksStoreArrayName);
}

void RecoTrackStoreArrayCombinerModule::initialize()
{
  m_temp1RecoTracks.isRequired(m_temp1RecoTracksStoreArrayName);
  m_temp2RecoTracks.isRequired(m_temp2RecoTracksStoreArrayName);

  m_recoTracks.registerInDataStore(m_recoTracksStoreArrayName, DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_recoTracks);

  m_recoTracks.registerRelationTo(m_temp1RecoTracks);
  m_recoTracks.registerRelationTo(m_temp2RecoTracks);
}

void RecoTrackStoreArrayCombinerModule::event()
{
  TrackFitter trackFitter;

  for (RecoTrack& temp1RecoTrack : m_temp1RecoTracks) {
    RecoTrack* newTrack = temp1RecoTrack.copyToStoreArray(m_recoTracks);
    newTrack->addHitsFromRecoTrack(&temp1RecoTrack, newTrack->getNumberOfTotalHits());
    newTrack->addRelationTo(&temp1RecoTrack);
  }

  for (RecoTrack& temp2RecoTrack : m_temp2RecoTracks) {
    RecoTrack* newTrack = temp2RecoTrack.copyToStoreArray(m_recoTracks);
    newTrack->addHitsFromRecoTrack(&temp2RecoTrack, newTrack->getNumberOfTotalHits());
    newTrack->addRelationTo(&temp2RecoTrack);
  }
}

