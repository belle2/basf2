/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>


namespace Belle2 {

  /**
   * TOP digitizer.
   * This module takes hits form G4 simulation (TOPSimHits),
   * applies TTS, T0 jitter and does spatial and time digitization.
   * (QE had been moved to the simulation: applied in SensitiveBar, SensitivePMT)
   * Output to TOPDigits.
   */
  class TOPDigitizerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPDigitizerModule();

    /**
     * Destructor
     */
    virtual ~TOPDigitizerModule();

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

    double m_timeZeroJitter = 0;       /**< r.m.s of T0 jitter */
    double m_darkNoise = 0;            /**< uniform dark noise (hits per bar) */
    double m_trigT0Sigma = 0;          /**< trigger T0 resolution */
    double m_bunchTimeSep = 0;         /**< time between two bunches */

  };

} // Belle2 namespace

