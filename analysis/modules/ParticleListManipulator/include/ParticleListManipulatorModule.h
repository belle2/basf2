/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <memory>

namespace Belle2 {

  // forward declaration
  class Particle;

  /**
   * Module for copying Particles (actually their indices) from
   * two or more ParticleLists(s) to another ParticleList. In the
   * process selection criteria can be applied, meaning that all
   * Particles that pass it will be copied.
   * Note that the Particles themselves are not copied.The original and copied
   * ParticleLists will point to the same Particles.
   */
  class ParticleListManipulatorModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleListManipulatorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    StoreArray<Particle> m_particles; /**< StoreArray of Particles */
    StoreObjPtr<ParticleList> m_particleList; /**< output particle list */
    StoreObjPtr<ParticleList> m_antiParticleList; /**< output anti-particle list */

    int m_pdgCode; /**< PDG code of the particles */

    std::vector<std::string> m_inputListNames; /**< input ParticleList names */
    std::string m_outputListName;              /**< output ParticleList name */
    DecayDescriptor m_decaydescriptor;         /**< Decay descriptor of the particle being selected */

    std::string m_outputAntiListName;   /**< output anti-particle list name */
    bool m_isSelfConjugatedParticle;    /**< flag that indicates whether an anti-particle does not exist and therefore the output anti-ParticleList should not be created */

    std::string m_cutParameter;  /**< selection criteria */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

    std::string m_variableName; /**< Variable which defines the best duplicate. */
    bool m_preferLowest; /**< Select the duplicate with the lowest value (instead of highest). */
    const Variable::Manager::Var* m_variable; /**< Variable which defines the best duplicate selection. */

    bool m_writeOut;                     /**< toggle Particle List btw. transient/writeOut */

    /**
     * This vector holds unique identifiers (vector of ints) of all particles
     * that are already included in the ParticleList. It is used to prevent
     * adding one or more copies of a Particle to the list that is already in.
     */
    std::vector<std::vector<int>> m_particlesInTheList;


    /**
     * Fills unique identifier for the input particle.
     * The unique identifier is a sequence of pairs (PDG code of the particle,
     * number of daughters if composite or mdstArrayIndex if final state particle)
     * for all particles in the decay chain.
     *
     * Example: B+ -> (anti-D0 -> K+ pi-) pi+
     * idSequence: (521, 2, -421, 2, 321, K+ mdstArrayIndex, -211, pi- mdstArrayIndex, 211, pi+ mdstArrayIndex)
     */
    void fillUniqueIdentifier(const Particle* p, std::vector<int>& idSequence);

    /**
     * Compares input idSequence to all sequences already included in the list.
     * If sequence is not found in the list a sequence is found to be unique.
     */
    bool isUnique(const std::vector<int>& idSequence);

  };

} // Belle2 namespace

