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
    std::string m_param_trackCandidatesStoreArrayName = "TrackCands"; /**< StoreArray name of the input track candidates. */
    bool m_param_recreateSortingParameters = false; /**< Flag to recreate the sorting parameters of the hit out of the stored order. */

    /** StoreArray name of the input and output reco tracks. */
    std::string m_param_recoTracksStoreArrayName = "RecoTracks";
    /** StoreArray name of the PXD hits. */
    std::string m_param_pxdHitsStoreArrayName = "";
    /** StoreArray name of the SVD hits. */
    std::string m_param_svdHitsStoreArrayName = "";
    /** StoreArray name of the CDC hits. */
    std::string m_param_cdcHitsStoreArrayName = "";
    /** StoreArray name of the BKLM hits. */
    std::string m_param_bklmHitsStoreArrayName = "";
    /** StoreArray name of the EKLM hits. */
    std::string m_param_eklmHitsStoreArrayName = "";
    /** StoreArray name of the output reco hit information. */
    std::string m_param_recoHitInformationStoreArrayName = "";
  };
}

