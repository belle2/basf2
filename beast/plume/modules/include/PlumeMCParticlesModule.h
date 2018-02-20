/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PLUMEMCPARTICLESMODULE_H
#define PLUMEMCPARTICLESMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <TFile.h>
#include <TNtuple.h>

namespace Belle2 {

  /**
   *
   */
  class PLUMEMCParticlesModule : public Module {

  public:

    /**
     * Constructor
     */
    PLUMEMCParticlesModule();

    /**
     * Destructor
     */
    virtual ~PLUMEMCParticlesModule();

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

  };

} // Belle2 namespace

#endif
