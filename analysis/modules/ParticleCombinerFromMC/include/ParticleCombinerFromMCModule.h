/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <string>
#include <vector>
#include <memory>

namespace Belle2 {
  class Particle;

  /**
   * particle combiner module
   */
  class ParticleCombinerFromMCModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleCombinerFromMCModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * Register particles which have (sub-)decay recursively.
     * The function is called in initialize().
     */
    void registerParticleRecursively(const DecayDescriptor& decaydescriptor);

    /**
     * Combine particles which have (sub-)decay recursively.
     * The function is called in event().
     */
    void combineRecursively(const DecayDescriptor& decaydescriptor);


  private:

    StoreArray<Particle> m_particles; /**< StoreArray of Particles */

    int m_pdgCode;                /**< PDG code of the combined mother particle */

    std::string m_decayString;   /**< Input DecayString specifying the decay being reconstructed */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the decay being reconstructed */
    std::unique_ptr<ParticleGenerator> m_generator; /**< Generates the combinations */

    std::string m_listName;   /**< output particle list name */
    std::string m_antiListName;   /**< output anti-particle list name */

    int m_decayModeID; /**< user specified decay mode identifier */

    std::vector<std::string> m_vector_listName;   /**< vector of output particle list name */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle mother does not exist and should not be reconstructed as well*/

    std::string m_cutParameter;  /**< selection criteria */

    std::unique_ptr<Variable::Cut> m_cut;  /**< cut object which performs the cuts */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

    bool m_chargeConjugation; /**< boolean to control whether charge conjugated decay should be reconstructed as well */

  };

} // Belle2 namespace

