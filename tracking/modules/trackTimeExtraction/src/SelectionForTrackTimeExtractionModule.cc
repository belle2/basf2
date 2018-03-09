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
  addParam("selectionCriteria", m_param_selectionCriteria, "Determines which quantity is used to select the best "
           "track for time extraction. Must be one of: highest_pt, most_hits, cosmics_lower_segment, "
           "cosmics_upper_segment", m_param_selectionCriteria);

}

void SelectionForTrackTimeExtractionModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<RecoTrack> selectedRecoTracks(m_param_selectedRecoTracksStoreArrayName);
  selectedRecoTracks.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

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

    std::function < bool (RecoTrack*, RecoTrack*)> lmdSort;

    // selects the track with the highest pt, this is meaningless in cases
    // where there is no magnetic field and pt cannot be measured properly
    if (m_param_selectionCriteria == "highest_pt") {
      lmdSort = [](RecoTrack * lhs, RecoTrack * rhs) {
        return (lhs->getMomentumSeed().Pt() > rhs->getMomentumSeed().Pt());
      };
    }
    // select the track with the most hits
    else if (m_param_selectionCriteria == "most_hits") {
      lmdSort = [](RecoTrack * lhs, RecoTrack * rhs) {
        return (lhs->getNumberOfCDCHits() > rhs->getNumberOfCDCHits());
      };
      // select tracks in the lower half of the cdc. This can be especially useful
      // for CDC cosmic muon tracks
    } else if (m_param_selectionCriteria == "cosmics_lower_segment") {
      lmdSort = [](RecoTrack * lhs, RecoTrack * rhs) {
        return (lhs->getPositionSeed().Y() < rhs->getPositionSeed().Y());
      };
      // select tracks in the upper half of the cdc. This can be especially useful
      // for CDC cosmic muon tracks
    } else if (m_param_selectionCriteria == "cosmics_upper_segment") {
      lmdSort = [](RecoTrack * lhs, RecoTrack * rhs) {
        return (lhs->getPositionSeed().Y() > rhs->getPositionSeed().Y());
      };
    } else {
      B2FATAL("Selection criteria " + m_param_selectionCriteria + " not supported.");
    }

    std::sort(recoTracks.begin(), recoTracks.end(), lmdSort);

    for (unsigned int trackCounter = 0;
         trackCounter < std::min(recoTracks.size(), m_param_maximalNumberOfTracks); trackCounter++) {
      RecoTrack* maximumPtRecoTrack = recoTracks[trackCounter];

      RecoTrack* selectedRecoTrack = maximumPtRecoTrack->copyToStoreArray(selectedRecoTracks);
      selectedRecoTrack->addHitsFromRecoTrack(maximumPtRecoTrack);
    }

    if (m_param_deleteOtherRecoTracks) {
      // Delete the other RecoTracks, as they were probably found under a wrong T0 assumption.
      recoTracks.clear();
    }
  }
}
