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
  class AbsFitter;
}

namespace Belle2 {

  class BaseRecoFitterModule : public Module {

  public:
    /** Constructor .
     */
    BaseRecoFitterModule();

    /** Empty destructor. */
    ~BaseRecoFitterModule() { }

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize() override;

    /** Called when entering a new run.
     */
    void beginRun() override { }

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event() override;

    /** This method is called if the current run ends.
     */
    void endRun() override { }

    /** This method is called at the end of the event processing.
     */
    void terminate() override { }

  protected:
    /** Method do create the used filter
     * This method has to by implemented by the derived class
     * @return The fitter we will use for fitting.
     */
    virtual std::shared_ptr<genfit::AbsFitter> createFitter() const = 0;

  private:
    std::string m_param_recoTracksStoreArrayName; /**< StoreArray name of the input and output reco tracks */
  };
}

