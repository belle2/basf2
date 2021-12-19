/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackCreationHelper/TrackStoreArrayHelper.h>


namespace Belle2 {
  void TrackStoreArrayHelper::registerArray(const std::string& arrayName, bool errorIfAlreadyRegistered)
  {
    m_recoTracksStoreArrayName = arrayName;
    StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);

    if (errorIfAlreadyRegistered) {
      recoTracks.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
    } else {
      recoTracks.registerInDataStore();
    }
    RecoTrack::registerRequiredRelations(recoTracks);
  }

  void TrackStoreArrayHelper::copyTrackWithHitsAndRelations(const RecoTrack& recoTrack)
  {
    StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
    RecoTrack* newRecoTrack = recoTrack.copyToStoreArray(recoTracks);
    newRecoTrack->addHitsFromRecoTrack(&recoTrack);
    newRecoTrack->addRelationTo(&recoTrack);
  }
}
