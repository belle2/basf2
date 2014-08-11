/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLECOMBINERMODULE_H
#define PARTICLECOMBINERMODULE_H

#include <framework/core/Module.h>

#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <vector>
#include <tuple>


namespace Belle2 {
  class Particle;

  /**
   * particle combiner module
   */
  class ParticleCombinerModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleCombinerModule();

    /**
     * Destructor
     */
    virtual ~ParticleCombinerModule();

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

    int m_pdgCode;                /**< PDG code of the combined mother particle */

    std::string m_decayString;   /**< Input DecayString specifying the decay being reconstructed */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the decay being reconstructed */
    ParticleGenerator* m_generator; /**< Generates the combinations */

    std::string m_listName;   /**< output particle list name */
    std::string m_antiListName;   /**< output anti-particle list name */

    int m_decayModeID; /**< user specified decay mode identifier */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle mother does not exist and should not be reconstructed as well*/

    Variable::Cut::Parameter m_cutParameter;  /**< selection criteria */
    Variable::Cut m_cut; /**< cut object which performs the cuts */

    bool m_persistent;  /**< toggle output particle list btw. transient/persistent */

  };

} // Belle2 namespace

#endif
