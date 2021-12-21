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

    if (errorIfAlreadyRegistered) {
      m_recoTracks.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
    } else {
      m_recoTracks.registerInDataStore();
    }
    RecoTrack::registerRequiredRelations(m_recoTracks);
  }

  void TrackStoreArrayHelper::copyTrackWithHitsAndRelations(const RecoTrack& recoTrack)
  {
    RecoTrack* newRecoTrack = recoTrack.copyToStoreArray(m_recoTracks);
    newRecoTrack->addHitsFromRecoTrack(&recoTrack);
    newRecoTrack->addRelationTo(&recoTrack);
  }
}
