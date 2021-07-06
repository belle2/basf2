/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
   * Module use to select two cosmic tracks event and merger these two tracks become one.
   * If the deleteOtherRecoTracks flag is on, it will clean up the
   * input store array afterwards.
   */
  class CDCCosmicTrackMergerModule : public Module {
  public:
    /// Create a new instance of the module.
    CDCCosmicTrackMergerModule();

    /// Register the store arrays and store obj pointers.
    void initialize() override;

    /// Do the selection.
    void event() override;

  private:
    /// StoreArray name from which to read the reco tracks.
    std::string m_param_recoTracksStoreArrayName = "";
    /// StoreArray name where the merged reco track is written.
    std::string m_param_MergedRecoTracksStoreArrayName = "__MergedRecoTracks";
    /// Flag to delete the not RecoTracks from the input StoreArray.
    bool m_param_deleteOtherRecoTracks = true;
    /// Minium NDF required for each track (up and down).
    double m_MinimumNumHitCut = 40;
    /// Tracks.
    StoreArray<RecoTrack> m_RecoTracks;
    /// Merged tracks.
    StoreArray<RecoTrack> m_MergedRecoTracks;
  };
}
