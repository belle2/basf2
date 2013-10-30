/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPBEAMTRIGGERMODULE_H
#define TOPBEAMTRIGGERMODULE_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * emulation of test beam trigger
   * module returns true if trigger fires
   */
  class TOPbeamTriggerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPbeamTriggerModule();

    /**
     * Destructor
     */
    virtual ~TOPbeamTriggerModule();

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

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    std::vector<int> m_detectorIDs;   /**< list of detector ID's included in trigger */
    std::vector<double> m_thresholds; /**< list of detector thresholds */

  };

} // Belle2 namespace

#endif
