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

#include <analysis/VariableManager/Utility.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <framework/core/Module.h>

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

    int m_pdgCode; /**< PDG code of the particles */

    std::vector<std::string> m_inputListNames; /**< input ParticleList names */
    std::string m_outputListName;              /**< output ParticleList name */
    DecayDescriptor m_decaydescriptor;         /**< Decay descriptor of the particle being selected */

    std::string m_outputAntiListName;   /**< output anti-particle list name */
    bool m_isSelfConjugatedParticle;    /**< flag that indicates whether an anti-particle does not exist and therefore the output anti-ParticleList should not be created */

    std::string m_cutParameter;  /**< selection criteria */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

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

    /**
     * Compares two sequences and returns true if they are found to be the same.
     */
    bool isIdenticalSequence(const std::vector<int>& idSeqIN, const std::vector<int>& idSeqOUT);

  };

} // Belle2 namespace

