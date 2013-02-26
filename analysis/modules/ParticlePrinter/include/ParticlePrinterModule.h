/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEPRINTERMODULE_H
#define PARTICLEPRINTERMODULE_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * prints particle list to screen
   */
  class ParticlePrinterModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticlePrinterModule();

    /**
     * Destructor
     */
    virtual ~ParticlePrinterModule();

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

    std::string m_listName;   /**< particle list name */
    bool m_fullPrint;         /**< steering variable */

  };

} // Belle2 namespace

#endif
