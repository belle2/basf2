/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPSimCalPulse.h>
#include <vector>

namespace Belle2 {

  /**
   * Generator of calibration pulses
   * Output to TOPSimCalPulses
   *
   * Needs TOPDigitizer configured to full waveform digitization to digitize its output
   */
  class TOPCalPulseGeneratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCalPulseGeneratorModule();

    /**
     * Destructor
     */
    virtual ~TOPCalPulseGeneratorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    // module steering parameters

    std::vector<int> m_moduleIDs; /**< slot ID's to generate for */
    std::vector<unsigned> m_asicChannels; /**< ASIC calibration channels */
    double m_amplitude; /**< cal pulse amplitude [ADC counts] */

    // dataobjects
    StoreArray<TOPSimCalPulse> m_calPulses; /**< collection of simulated cal pulses */


  };

} // Belle2 namespace

