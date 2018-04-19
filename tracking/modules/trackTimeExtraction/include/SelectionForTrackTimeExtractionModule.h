/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * Module needed for the track time extraction module. It selects one or more RecoTrack from the
   * given store array, which is fittable and has the maximum pt and copies them over to the
   * store array of selected reco tracks. If the deleteOtherRecoTracks flag is on, it will clean up the
   * input store array afterwards.
   */
  class SelectionForTrackTimeExtractionModule final : public Module {
  public:
    /// Create a new instance of the module.
    SelectionForTrackTimeExtractionModule();

    /// Register the store arrays and store obj pointers.
    void initialize() override;

    /// Do the selection.
    void event() override;

  private:
    /// StoreArray name from which to read the reco tracks.
    std::string m_param_recoTracksStoreArrayName = "";
    /// StoreArray name to which to write the selected reco tracks.
    std::string m_param_selectedRecoTracksStoreArrayName = "__SelectedRecoTracks";
    /// Parameter to choose the selection criteria of the best tracks
    std::string m_param_selectionCriteria = "highest_pt";
    /// Flag to delete the not selected RecoTracks from the input StoreArray.
    bool m_param_deleteOtherRecoTracks = true;
    /// How many reco tracks should be copied over.
    unsigned long m_param_maximalNumberOfTracks = 2;
  };
}
