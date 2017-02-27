/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackTimeExtraction/SelectionForTrackTimeExtractionModule.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <framework/datastore/StoreArray.h>

using namespace Belle2;

REG_MODULE(SelectionForTrackTimeExtraction);


SelectionForTrackTimeExtractionModule::SelectionForTrackTimeExtractionModule() : Module()
{
  setDescription("Select RecoTracks for later track time extraction and copy them to another store array."
                 "Will clean the old reco tracks store array afterwards, if the parameter is set to do so.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName,
           "StoreArray containing the RecoTracks to read from and delete afterwards.",
           m_param_recoTracksStoreArrayName);
  addParam("selectedRecoTracksStoreArrayName", m_param_selectedRecoTracksStoreArrayName,
           "StoreArray to where to copy the selected RecoTracks.",
           m_param_selectedRecoTracksStoreArrayName);
  addParam("deleteOtherRecoTracks", m_param_deleteOtherRecoTracks,
           "Flag to delete the not selected RecoTracks from the input StoreArray.",
           m_param_deleteOtherRecoTracks);
  addParam("maximalNumberOfTracks", m_param_maximalNumberOfTracks, "How many reco tracks should be copied over.",
           m_param_maximalNumberOfTracks);

}

void SelectionForTrackTimeExtractionModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<RecoTrack> selectedRecoTracks(m_param_selectedRecoTracksStoreArrayName);
  selectedRecoTracks.registerInDataStore();

  RecoTrack::registerRequiredRelations(selectedRecoTracks);
}

void SelectionForTrackTimeExtractionModule::event()
{
  StoreArray<RecoTrack> recoTrackStoreArray(m_param_recoTracksStoreArrayName);
  StoreArray<RecoTrack> selectedRecoTracks(m_param_selectedRecoTracksStoreArrayName);

  if (recoTrackStoreArray.getEntries() > 0) {

    std::vector<RecoTrack*> recoTracks;
    recoTracks.reserve(static_cast<unsigned int>(recoTrackStoreArray.getEntries()));

    for (RecoTrack& recoTrack : recoTrackStoreArray) {
      if (recoTrack.wasFitSuccessful()) {
        recoTracks.push_back(&recoTrack);
      }
    }

    std::sort(recoTracks.begin(), recoTracks.end(), [](RecoTrack * lhs,
    RecoTrack * rhs) {
      return (lhs->getMomentumSeed().Pt() > rhs->getMomentumSeed().Pt());
    });

    for (unsigned int trackCounter = 0;
         trackCounter < std::min(recoTracks.size(), m_param_maximalNumberOfTracks); trackCounter++) {
      RecoTrack* maximumPtRecoTrack = recoTracks[trackCounter];
      RecoTrack* selectedRecoTrack = selectedRecoTracks.appendNew(maximumPtRecoTrack->getPositionSeed(),
                                                                  maximumPtRecoTrack->getMomentumSeed(),
                                                                  maximumPtRecoTrack->getChargeSeed());

      // retain the seed time of the original track. Important for t0 extraction.
      selectedRecoTrack->setTimeSeed(maximumPtRecoTrack->getTimeSeed());
      selectedRecoTrack->addHitsFromRecoTrack(maximumPtRecoTrack);
    }

    if (m_param_deleteOtherRecoTracks) {
      // Delete the other RecoTracks, as they were probably found under a wrong T0 assumption.
      recoTracks.clear();
    }
  }
}
