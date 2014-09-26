/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEPRINTERMODULE_H
#define PARTICLEPRINTERMODULE_H

#include <framework/core/Module.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <string>

#include <analysis/dataobjects/Particle.h>

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

  private:

    std::string m_listName;   /**< particle list name */
    bool m_fullPrint;         /**< steering variable */

    std::vector<std::string> m_variables;  /**< names of variables to be printed */

    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the particle being selected */

    /**
     * Prints out the values of variables specified by the user via the Variables module parameter
     */
    void printVariables(const Particle* particle) const;
  };

} // Belle2 namespace

#endif
