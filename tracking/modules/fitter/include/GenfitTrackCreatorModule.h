/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /** Module turning reco tracks to genfit tracks (will be unneeded once we use reco tracks everywhere). */
  class GenfitTrackCreatorModule : public Module {

  public:
    /** Initialize the module parameters. */
    GenfitTrackCreatorModule();

    /** Initialize the needed StoreArrays and ensure they are created properly. */
    void initialize() override;

    /** Loop over all reco tracks and create a genfit track. */
    void event() override;

  private:
    std::string m_param_genfitTrackStoreArrayName = "GF2Tracks"; /**< StoreArray name of the output genfit tracks. */
    std::string m_param_recoTracksStoreArrayName = "RecoTracks"; /**< StoreArray name of the input reco tracks. */
    std::string m_param_genfitTrackCandsStoreArrayName = "TrackCands"; /**< StoreArray name of the related track candidates. */
  };
}

