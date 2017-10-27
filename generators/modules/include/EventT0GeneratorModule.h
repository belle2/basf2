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
#include <string>

namespace Belle2 {

  /**
   * Emulation of L1 t0
   */
  class EventT0GeneratorModule : public Module {

  public:

    /**
     * Constructor
     */
    EventT0GeneratorModule();

    /**
     * Destructor
     */
    virtual ~EventT0GeneratorModule();

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
    double m_coreGaussWidth; /**< sigma of core gaussian [ns] */
    double m_tailGaussWidth; /**< sigma of tail gaussian [ns] */
    double m_tailGaussFraction; /**< area fraction of core gaussian */

    // other
    double m_bunchTimeSep = 0;         /**< time between two bunches */


  };

} // Belle2 namespace

