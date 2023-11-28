/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {
  /** Computes the track time, defined as the difference between the average of SVD clusters time and the SVDEvent T0 */
  class TrackTimeEstimatorModule : public Module {

  public:

    TrackTimeEstimatorModule();
    /** Verify that required objects are registered */
    void initialize();

    /** Loop over all Tracks, get related recoTrack, get SVDHitlist, compute average time, set trackTime */
    void event();


  private:
    /// Accessing the recoTracks, they have the SVD Hits attached
    StoreArray<RecoTrack> m_recoTracks;
    /// Accessing the Tracks
    StoreArray<Track> m_tracks;
    /// Accessing eventT0, as we want SVDEventT0
    StoreObjPtr<EventT0> m_evtT0;
  };
}
