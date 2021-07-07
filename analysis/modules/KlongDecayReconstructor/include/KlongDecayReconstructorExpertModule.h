/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/VariableManager/Utility.h>
#include <analysis/ParticleCombiner/ParticleCombiner.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <string>

namespace Belle2 {
  class Particle;

  /**
   * reco missing module
   */
  class KlongDecayReconstructorExpertModule : public Module {

  public:

    /**
     * Constructor
     */
    KlongDecayReconstructorExpertModule();

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

    StoreObjPtr<ParticleList> m_outputList; /**< output particle list */
    StoreObjPtr<ParticleList> m_outputAntiList; /**< output anti-particle list */

    int m_pdgCode;                /**< PDG code of the combined mother particle */

    std::string m_decayString;   /**< Input DecayString specifying the decay being reconstructed */
    std::string m_finalDecayString;   /**< Final DecayString specifying the decay being reconstructed with recalculated KL momenta*/
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the decay being reconstructed */
    std::unique_ptr<ParticleGenerator> m_generator; /**< Generates the combinations */

    std::string m_listName;   /**< output particle list name */
    std::string m_antiListName;   /**< output anti-particle list name */

    std::string m_recoList;   /**< suffix for input K_L0 list name */

    int m_decayModeID; /**< user specified decay mode identifier */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle mother does not exist and should not be reconstructed as well*/

    std::string m_cutParameter;  /**< selection criteria */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

    int m_maximumNumberOfCandidates; /**< drop all candidates if more candidates than this parameter are produced */

  };

} // Belle2 namespace

//#endif
