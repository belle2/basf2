/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yo Sato                                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

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
  class NewMCDecayFinderModule : public Module {

  public:

    /**
     * Constructor
     */
    NewMCDecayFinderModule();

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
    void registerParticleRecursively(DecayDescriptor decaydescriptor);

    /**
     * Combine particles which have (sub-)decay recursively.
     * The function is called in event().
     */
    void combineRecursively(DecayDescriptor decaydescriptor);


  private:

    int m_pdgCode;                /**< PDG code of the combined mother particle */

    std::string m_decayString;   /**< Input DecayString specifying the decay being reconstructed */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the decay being reconstructed */
    std::unique_ptr<ParticleGenerator> m_generator; /**< Generates the combinations */

    std::string m_listName;   /**< output particle list name */
    std::string m_antiListName;   /**< output anti-particle list name */

    std::vector<std::string> m_vector_listName;   /**< vector of output particle list name */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle mother does not exist and should not be reconstructed as well*/

    std::string m_cutParameter;  /**< selection criteria */

    std::unique_ptr<Variable::Cut> m_cut;  /**< cut object which performs the cuts */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

  };

} // Belle2 namespace

