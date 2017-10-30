/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Digitizer that provides time stamps for TOP trigger
   */
  class TOPTriggerDigitizerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPTriggerDigitizerModule();

    /**
     * Destructor
     */
    virtual ~TOPTriggerDigitizerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Various enums
     */
    enum {c_SamplingCycle = 8, /**< timestamp sampling period [samples] */
          c_Frame9Period = 30720 /**< number of sampling cycles per frame9 marker */
         };

    // module steering parameters
    int m_threshold;  /**< pulse height threshold [ADC counts] */
    int m_hysteresis; /**< pulse height threshold hysteresis [ADC counts] */
    int m_gateWidth;  /**< width of discriminator gate [samples] */
    int m_samplingPhase;  /**< sampling phase [samples] */
    int m_bunchTimeStamp; /**< time stamp of the interaction (see addParam for more) */
  };

} // Belle2 namespace

