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

namespace genfit {
  class AbsFitter;
}


namespace Belle2 {

  class RecoTrack;

  /** A base class for all modules that implement a fitter for reco tracks. */
  class BaseRecoFitterModule : public Module {

  public:
    /**
     * Constructor.
     */
    BaseRecoFitterModule();

    /**
     * Initialize the store ararys and check for the material effects.
     */
    void initialize() override;

    /**
     * Do the fitting using the created fitter.
     */
    void event() override;


  protected:
    /**
     * Method to create the used filter.
     * This method has to by implemented by the derived class
     *
     * If you return a nullptr here, it means the default fitter (defined by the TrackFitter)
     * is used.
     *
     * @return The fitter we will use for fitting.
     */
    virtual std::shared_ptr<genfit::AbsFitter> createFitter() const = 0;

  private:
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
    /** Use these particle hypotheses for fitting. Please use positive pdg codes only. */
    std::vector<unsigned int> m_param_pdgCodesToUseForFitting = {211};
    /** Resort the hits while fitting. */
    bool m_param_resortHits = false;
    /** Configures whether the CDC Translators should be initialized by the FitterModule
     * especially useful for VXD-only beamtest. In the future this could be changed to check
     * implicitly if the cdc is available in the geometry.*/
    bool m_param_initializeCDCTranslators = true;
  };
}

