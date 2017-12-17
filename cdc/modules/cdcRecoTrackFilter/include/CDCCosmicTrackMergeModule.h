/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:    Dong Van Thanh                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/core/Module.h>

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
    std::string m_param_mergedRecoTracksStoreArrayName = "__MergedRecoTracks";
    /// Flag to delete the not RecoTracks from the input StoreArray.
    bool m_param_deleteOtherRecoTracks = true;
    /// Minium NDF required for each track (up and down).
    double m_minimumNumHitCut = 40;
  };
}
