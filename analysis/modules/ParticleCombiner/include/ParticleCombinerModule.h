/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <string>
#include <memory>

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
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the decay being reconstructed */
    std::unique_ptr<ParticleGenerator> m_generator; /**< Generates the combinations */

    std::string m_listName;   /**< output particle list name */
    std::string m_antiListName;   /**< output anti-particle list name */

    int m_decayModeID; /**< user specified decay mode identifier */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle mother does not exist and should not be reconstructed as well*/

    std::string m_cutParameter;  /**< selection criteria */

    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

    int m_recoilParticleType =
      0; /**< type of recoil particle: 0 - not recoil (normal reconstruction); 1 - recoil against e+e- and all daughters; 2 - recoil against first daughter */

    int m_maximumNumberOfCandidates; /**< maximum number of reconstructed candidates */

    bool m_ignoreIfTooManyCandidates;  /**< drop all candidates if max. number of candidate reached */

    bool m_chargeConjugation; /**< boolean to control whether charge conjugated decay should be reconstructed as well */

    bool m_allowChargeViolation; /**< switch to turn on and off the requirement of electric charge conservation */

  };

} // Belle2 namespace

