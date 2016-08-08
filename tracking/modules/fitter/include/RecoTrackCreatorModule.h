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

  /** Module turning trackCands to reco tracks (will be unneeded once we create reco tracks from the beginning) */
  class RecoTrackCreatorModule : public Module {

  public:
    /** Initilialize the module parameters. */
    RecoTrackCreatorModule();

    /** Initialize the needed StoreArrays and ensure they are created properly. */
    void initialize() override;

    /** Loop over all track candidates and create a recotrack. */
    void event() override;

  private:
    std::string m_param_trackCandidatesStoreArrayName = ""; /**< StoreArray name of the input track candidates. */
    std::string m_param_recoTracksStoreArrayName = ""; /**< StoreArray name of the output reco tracks. */
    std::string m_param_recoHitInformationStoreArrayName = ""; /**< StoreArray name of the output reco hit information. */
    bool m_param_recreateSortingParameters = false; /**< Flag to recreate the sorting parameters of the hit out of the stored order. */

    std::string m_param_cdcHitsStoreArrayName = ""; /**< StoreArray name of the input cdc hits. */
    std::string m_param_svdHitsStoreArrayName = ""; /**< StoreArray name of the input svd hits. */
    std::string m_param_pxdHitsStoreArrayName = ""; /**< StoreArray name of the input pxd hits. */
  };
}

