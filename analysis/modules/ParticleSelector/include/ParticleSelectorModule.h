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
#include <analysis/VariableManager/Utility.h>
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
    std::string m_antiListName;   /**< output anti-particle list name */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle does not exist */

    int m_pdgCode;       /**< PDG code of particles to select */

    Variable::Cut::Parameter m_cutParameter;  /**< selection criteria */
    Variable::Cut m_cut; /**< cut object which performs the cuts */

    bool m_writeOut;  /**< toggle particle list btw. transient/writeOut */

  };

} // Belle2 namespace

#endif
