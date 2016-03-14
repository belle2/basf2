/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
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
  class AbsKalmanFitter;
}


namespace Belle2 {

  class RecoTrack;

  class BaseRecoFitterModule : public Module {

  public:
    /**
     * Constructor .
     */
    BaseRecoFitterModule();

    /**
     * Initialize the store ararys and check for the material effects.
     */
    void initialize() override;

    /**
     * Do the fitting using the created fitter
     */
    void event() override;


  protected:
    /**
     * Method do create the used filter
     * This method has to by implemented by the derived class
     * @return The fitter we will use for fitting.
     */
    virtual std::shared_ptr<genfit::AbsKalmanFitter> createFitter() const = 0;

  private:
    /** StoreArray name of the input and output reco tracks */
    std::string m_param_recoTracksStoreArrayName = "RecoTracks";
    /** Use this particle hypothesis for fitting. Please use the positive pdg code only. */
    unsigned int m_param_pdgCodeToUseForFitting = 211;
    /** Resort the hits while fitting */
    bool m_param_resortHits = false;
    /** Maximum number of failed hits before aborting the fit */
    unsigned int m_param_maxNumberOfFailedHits = 5;
  };
}

