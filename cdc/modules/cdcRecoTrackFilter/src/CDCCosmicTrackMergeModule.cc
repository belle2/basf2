/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  m_RecoTracks.isRequired(m_param_recoTracksStoreArrayName);
  m_MergedRecoTracks.registerInDataStore(m_param_MergedRecoTracksStoreArrayName);
  RecoTrack::registerRequiredRelations(m_MergedRecoTracks);
}

void CDCCosmicTrackMergerModule::event()
{
  if (m_RecoTracks.getEntries() == 2) {
    if (m_RecoTracks[0]->getNumberOfCDCHits() > m_MinimumNumHitCut
        && m_RecoTracks[1]->getNumberOfCDCHits() > m_MinimumNumHitCut) {
      //  if(recoTrackStoreArray[0].getPositionSeed().Y() * recoTrackStoreArray[1].getPositionSeed().Y() >0) continue;

      std::vector<RecoTrack*> recoTracks;
      recoTracks.reserve(static_cast<unsigned int>(m_RecoTracks.getEntries()));

      for (RecoTrack& recoTrack : m_RecoTracks) {
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
      RecoTrack* MergedRecoTrack = m_MergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
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
