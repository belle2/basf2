/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: B.Oberhof, benjamin.oberhof@lnf.infn.it                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>

namespace Belle2 {
  class Particle;

  /**
   * reco missing module
   */
  class KLMomentumCalculatorModule : public Module {

  public:

    /**
     * Constructor
     */
    KLMomentumCalculatorModule();

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

    int m_pdgCode;                /**< PDG code of the combined mother particle */
    int m_kpdgCode;                /**< PDG code for the output K_L0 list */

    std::string m_decayString;   /**< Input DecayString specifying the decay being reconstructed */
    std::string m_finalDecayString;   /**< Final DecayString specifying the decay being reconstructed with recalculated KL momenta*/
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the decay being reconstructed */
    std::unique_ptr<ParticleGenerator> m_generator; /**< Generates the combinations */

    std::string m_listName;   /**< output particle list name */
    std::string m_antiListName;   /**< output anti-particle list name */

    std::string m_klistName;   /**< output K_L0 particle list name */

    int m_decayModeID; /**< user specified decay mode identifier */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle mother does not exist and should not be reconstructed as well*/

    std::string m_cutParameter;  /**< selection criteria */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

    std::string m_recoList; /**< Suffix attached to the output K_L list, if not defined it is set to '_reco'  */


    int m_maximumNumberOfCandidates; /**< drop all candidates if more candidates than this parameter are produced */

  };

} // Belle2 namespace

//#endif
