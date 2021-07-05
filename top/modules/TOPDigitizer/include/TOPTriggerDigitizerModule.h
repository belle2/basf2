/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

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
  };

} // Belle2 namespace

