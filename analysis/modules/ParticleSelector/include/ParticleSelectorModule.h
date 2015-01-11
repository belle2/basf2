/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLESELECTORMODULE_H
#define PARTICLESELECTORMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <analysis/VariableManager/Utility.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {

  /**
   * Loops over all Particles in the ParticleList and removes those Particles
   * from the ParticleList that do not pass given selection criteria.
   */
  class ParticleSelectorModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleSelectorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Event processor.
     */
    virtual void event();

  private:

    std::string m_decayString;   /**< Input DecayString specifying the particle being selected */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the particle being selected */

    std::string m_listName;   /**< output particle list name */

    Variable::Cut::Parameter m_cutParameter;  /**< selection criteria */
    Variable::Cut m_cut; /**< cut object which performs the cuts */
  };

} // Belle2 namespace

#endif
