/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/relatedTracksCombiner/RecoTrackStoreArrayCombiner.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

using namespace Belle2;

REG_MODULE(RecoTrackStoreArrayCombiner);

RecoTrackStoreArrayCombinerModule::RecoTrackStoreArrayCombinerModule() :
  Module()
{
  setDescription("Combine two collections of tracks without additional checks.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("Temp1RecoTracksStoreArrayName", m_temp1RecoTracksStoreArrayName, "Name of the first input StoreArray.",
           m_temp1RecoTracksStoreArrayName);
  addParam("Temp2RecoTracksStoreArrayName", m_temp2RecoTracksStoreArrayName, "Name of the second input StoreArray.",
           m_temp2RecoTracksStoreArrayName);
  addParam("Temp1SPTrackCandsStoreArrayName", m_temp1SPTrackCandsStoreArrayName,
           "Name of the SPTrackCands related with first StoreArray.",
           m_temp1SPTrackCandsStoreArrayName);
  addParam("Temp2SPTrackCandsStoreArrayName", m_temp2SPTrackCandsStoreArrayName,
           "Name of the SPTrackCands related with second StoreArray.",
           m_temp2SPTrackCandsStoreArrayName);
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the output StoreArray.", m_recoTracksStoreArrayName);
}

void RecoTrackStoreArrayCombinerModule::initialize()
{
  m_temp1RecoTracks.isRequired(m_temp1RecoTracksStoreArrayName);
  m_temp2RecoTracks.isRequired(m_temp2RecoTracksStoreArrayName);

  m_recoTracks.registerInDataStore(m_recoTracksStoreArrayName);
  RecoTrack::registerRequiredRelations(m_recoTracks);

  m_recoTracks.registerRelationTo(m_temp1RecoTracks);
  m_recoTracks.registerRelationTo(m_temp2RecoTracks);

  if (m_temp1SPTrackCandsStoreArrayName != "" && m_temp2SPTrackCandsStoreArrayName != "") {
    m_temp1SPTrackCands.isRequired(m_temp1SPTrackCandsStoreArrayName);
    m_temp2SPTrackCands.isRequired(m_temp2SPTrackCandsStoreArrayName);
    m_recoTracks.registerRelationTo(m_temp1SPTrackCands);
    m_recoTracks.registerRelationTo(m_temp2SPTrackCands);
  }

}

void RecoTrackStoreArrayCombinerModule::event()
{
  TrackFitter trackFitter;

  for (RecoTrack& temp1RecoTrack : m_temp1RecoTracks) {
    RecoTrack* newTrack = temp1RecoTrack.copyToStoreArray(m_recoTracks);
    newTrack->addHitsFromRecoTrack(&temp1RecoTrack, newTrack->getNumberOfTotalHits());
    newTrack->addRelationTo(&temp1RecoTrack);
    if (m_temp1SPTrackCandsStoreArrayName != "") {
      const SpacePointTrackCand* newSPTrackCands1 = temp1RecoTrack.getRelated<SpacePointTrackCand>(m_temp1SPTrackCandsStoreArrayName);
      newTrack->addRelationTo(newSPTrackCands1);
    }

  }

  for (RecoTrack& temp2RecoTrack : m_temp2RecoTracks) {
    RecoTrack* newTrack = temp2RecoTrack.copyToStoreArray(m_recoTracks);
    newTrack->addHitsFromRecoTrack(&temp2RecoTrack, newTrack->getNumberOfTotalHits());
    newTrack->addRelationTo(&temp2RecoTrack);
    if (m_temp2SPTrackCandsStoreArrayName != "") {
      const SpacePointTrackCand* newSPTrackCands2 = temp2RecoTrack.getRelated<SpacePointTrackCand>(m_temp2SPTrackCandsStoreArrayName);
      newTrack->addRelationTo(newSPTrackCands2);
    }
  }
}

