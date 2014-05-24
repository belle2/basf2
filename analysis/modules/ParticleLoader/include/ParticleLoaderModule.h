/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLELOADERMODULE_H
#define PARTICLELOADERMODULE_H

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {

  /**
   * particle loader module
   */
  class ParticleLoaderModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleLoaderModule();

    /**
     * Destructor
     */
    virtual ~ParticleLoaderModule();

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

    /**
     * Loads MCParticles to StoreArray<Particle>
     */
    void loadFromMCParticles();

    /**
     * Loads reconstructed particles from mdst to StoreArray<Particle>
     */
    void loadFromReconstruction();

    bool m_useMCParticles;  /**< steering variable */

  };

} // Belle2 namespace

#endif
