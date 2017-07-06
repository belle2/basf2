/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:    Dong Van Thanh                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcRecoTrackFilter/CDCCosmicTrackMergeModule.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

REG_MODULE(CDCCosmicTrackMerger);

CDCCosmicTrackMergerModule::CDCCosmicTrackMergerModule() : Module()
{
  setDescription("Select cosmic events containing two tracks (up/down) and merge two tracks"
                 "Old reco tracks store array will be deleted afterwards, if the parameter is set to do so.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName,
           "StoreArray containing the RecoTracks to read from and delete afterwards.",
           m_param_recoTracksStoreArrayName);
  addParam("MergedRecoTracksStoreArrayName", m_param_MergedRecoTracksStoreArrayName,
           "StoreArray to where to copy the merged RecoTrack.",
           m_param_MergedRecoTracksStoreArrayName);
  addParam("deleteOtherRecoTracks", m_param_deleteOtherRecoTracks,
           "Flag to delete the not Merged RecoTracks from the input StoreArray.",
           m_param_deleteOtherRecoTracks);
  addParam("MinimumNumHitCut", m_MinimumNumHitCut, "Number of hit per track required for each track", m_MinimumNumHitCut);
}

void CDCCosmicTrackMergerModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<RecoTrack> MergedRecoTracks(m_param_MergedRecoTracksStoreArrayName);
  MergedRecoTracks.registerInDataStore();

  RecoTrack::registerRequiredRelations(MergedRecoTracks);
}

void CDCCosmicTrackMergerModule::event()
{
  StoreArray<RecoTrack> recoTrackStoreArray(m_param_recoTracksStoreArrayName);
  StoreArray<RecoTrack> MergedRecoTracks(m_param_MergedRecoTracksStoreArrayName);

  if (recoTrackStoreArray.getEntries() == 2) {
    if (recoTrackStoreArray[0]->getNumberOfCDCHits() > m_MinimumNumHitCut
        && recoTrackStoreArray[1]->getNumberOfCDCHits() > m_MinimumNumHitCut) {
      //  if(recoTrackStoreArray[0].getPositionSeed().Y() * recoTrackStoreArray[1].getPositionSeed().Y() >0) continue;

      std::vector<RecoTrack*> recoTracks;
      recoTracks.reserve(static_cast<unsigned int>(recoTrackStoreArray.getEntries()));

      for (RecoTrack& recoTrack : recoTrackStoreArray) {
        recoTracks.push_back(&recoTrack);
      }

      std::function < bool (RecoTrack*, RecoTrack*)> lmdSort = [](RecoTrack * lhs, RecoTrack * rhs) {
        return (lhs->getPositionSeed().Y() > rhs->getPositionSeed().Y());
      };
      std::sort(recoTracks.begin(), recoTracks.end(), lmdSort);
      RecoTrack* upperTrack = recoTracks[0];
      RecoTrack* lowerTrack = recoTracks[1];
      B2DEBUG(99, "upper track posSeed :" << upperTrack->getPositionSeed().Y());
      B2DEBUG(99, "Lowee track posSeed :" << lowerTrack->getPositionSeed().Y());
      RecoTrack* MergedRecoTrack = MergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
                                                              upperTrack->getMomentumSeed(),
                                                              upperTrack->getChargeSeed());
      // retain the seed time of the original track. Important for t0 extraction.
      MergedRecoTrack->setTimeSeed(upperTrack->getTimeSeed());
      MergedRecoTrack->addHitsFromRecoTrack(upperTrack);
      MergedRecoTrack->addHitsFromRecoTrack(lowerTrack);
      if (m_param_deleteOtherRecoTracks) {
        // Delete the other RecoTracks, as they were probably found under a wrong T0 assumption.
        recoTracks.clear();
      }
    }
  }
}
