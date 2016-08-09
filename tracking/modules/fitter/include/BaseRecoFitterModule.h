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

#include <tracking/trackFitting/fitter/base/TrackFitter.h>

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
     * Method do create the used filter.
     * This method has to by implemented by the derived class
     * @return The fitter we will use for fitting.
     */
    virtual std::shared_ptr<genfit::AbsFitter> createFitter() const = 0;

  private:
    /** StoreArray name of the input and output reco tracks. */
    std::string m_param_recoTracksStoreArrayName = "RecoTracks";
    /** StoreArray name of the CDC hits. */
    std::string m_param_cdcHitsStoreArrayName = "CDCHits";
    /** StoreArray name of the SVD hits. */
    std::string m_param_svdHitsStoreArrayName = "SVDClusters";
    /** StoreArray name of the PXD hits. */
    std::string m_param_pxdHitsStoreArrayName = "PXDClusters";
    /** Use these particle hypotheses for fitting. Please use positive pdg codes only. */
    std::vector<unsigned int> m_param_pdgCodesToUseForFitting = {211};
    /** Resort the hits while fitting. */
    bool m_param_resortHits = false;
    /** FIXME: Temporary fix for the cosmics data. */
    bool m_param_cosmicsTemporaryFix = false;
  };
}

