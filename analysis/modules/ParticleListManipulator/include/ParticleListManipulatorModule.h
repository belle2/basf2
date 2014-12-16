/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLELISTMANIPULATORMODULE_H
#define PARTICLELISTMANIPULATORMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <analysis/VariableManager/Utility.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {

  /**
   * Module for copying Particles (actually their indices) from
   * two or more ParticleLists(s) to another ParticleList. In the
   * process selection criteria can be applied, meaning that all
   * Particles that pass it will be copied.
   */
  class ParticleListManipulatorModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleListManipulatorModule();

    /**
     * Destructor
     */
    virtual ~ParticleListManipulatorModule();

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

    int m_pdgCode; /**< PDG code of the particles */

    std::vector<std::string> m_inputListNames; /**< input ParticleList names */
    std::string m_outputListName;              /**< output ParticleList name */
    DecayDescriptor m_decaydescriptor;         /**< Decay descriptor of the particle being selected */

    std::string m_outputAntiListName;   /**< output anti-particle list name */
    bool m_isSelfConjugatedParticle;    /**< flag that indicates whether an anti-particle does not exist and therefore the output anti-ParticleList should not be created */

    Variable::Cut::Parameter m_cutParameter;  /**< selection criteria */
    Variable::Cut m_cut; /**< cut object which performs the cuts */

    bool m_writeOut;                     /**< toggle Particle List btw. transient/writeOut */

  };

} // Belle2 namespace

#endif
