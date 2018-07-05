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
#include <framework/gearbox/Const.h>
#include <string>

namespace Belle2 {

  /**
   * TOP reconstruction module.
   */
  class TOPPDFDebuggerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPPDFDebuggerModule();

    /**
     * Destructor
     */
    virtual ~TOPPDFDebuggerModule();

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     *
     */
    virtual void event();

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();


  private:

    // Module steering parameters
    double m_minBkgPerBar = 0; /**< minimal assumed background photons per bar */
    double m_scaleN0 = 0;      /**< scale factor for N0 */
    double m_maxTime = 0;      /**< optional time limit for photons */
    double m_minTime = 0;      /**< optional time limit for photons */
    int m_writeNPdfs = 0;      /**< write out pdfs for the first N events */
    int m_writeNPulls = 0;     /**< write out pulls for the furst N events */
    long long m_iEvent = -1;   /**< count events in the current process */

    // others
    int m_debugLevel = 0;       /**< debug level from logger */

    // Masses of particle hypotheses

    double m_masses[Const::ChargedStable::c_SetSize];  /**< particle masses */
    int m_pdgCodes[Const::ChargedStable::c_SetSize];   /**< particle codes */

  };

} // Belle2 namespace

