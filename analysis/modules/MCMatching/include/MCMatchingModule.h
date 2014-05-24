/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCMATCHINGMODULE_H
#define MCMATCHINGMODULE_H

#include <framework/core/Module.h>
#include <string>


namespace Belle2 {

  /**
   * MC matching module: module performs MC matching (sets the relation Particle -> MCParticle) for all particles
   * and its (grand)^N-daughter particles in the user-specified ParticleList. The MC matching algorithm is implemented
   * in analysis/utility/mcParticleMatching.h
   */
  class MCMatchingModule : public Module {

  public:

    /**
     * Constructor
     */
    MCMatchingModule();

    /**
     * Destructor
     */
    virtual ~MCMatchingModule();


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
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.                                                                                          */
    virtual void terminate();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

  private:

    std::string m_listName;  /**< steering variable: name of the input ParticleList */

  };
}
#endif
