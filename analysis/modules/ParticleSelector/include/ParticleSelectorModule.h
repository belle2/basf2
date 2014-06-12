/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLESELECTORMODULE_H
#define PARTICLESELECTORMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <analysis/utility/PSelector.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {

  /**
   * particle selector module
   */
  class ParticleSelectorModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleSelectorModule();

    /**
     * Destructor
     */
    virtual ~ParticleSelectorModule();

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

    std::string m_decayString;   /**< Input DecayString specifying the particle being selected */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the particle being selected */

    std::string m_listName;   /**< output particle list name */
    std::string m_antiListName;   /**< output anti-particle list name */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle does not exist */

    int m_pdgCode;       /**< PDG code of particles to select */

    std::vector<std::string> m_selection;  /**< selection criteria */
    std::vector<std::string> m_otherLists; /**< other lists used as input */
    bool m_persistent;  /**< toggle particle list btw. transient/persistent */

    PSelector m_pSelector;                 /**< particle selector */
    static bool m_printVariables;   /**< print available selection variables */

  };

} // Belle2 namespace

#endif
