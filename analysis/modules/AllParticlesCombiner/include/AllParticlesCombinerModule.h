/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/VariableManager/Utility.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  /**
   * This module combines all particles of the provided list to one mother particle.
   *
   */
  class AllParticlesCombinerModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    AllParticlesCombinerModule();

    /** Register input and output data */
    virtual void initialize() override;

    /** process event */
    virtual void event() override;


  private:

    std::string m_inputListName;  /**< List of particles which are supposed to be combined */
    StoreObjPtr<ParticleList> m_inputList;  /**< input particle list */

    bool m_isSelfConjugatedParticle;  /**< flag that indicates whether an anti-particle mother does not exist and should not be reconstructed as well*/

    std::string m_cutString;  /**< Selection criteria */
    std::unique_ptr<Variable::Cut> m_cut;  /**< cut object which performs the cuts */
    bool m_writeOut;  /**< If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file. */

    std::string m_outputListName;  /**< Name of the output list created by the combination of all particles in the input list. */
    std::string m_antiListName;  /**< output anti-particle list name */
    DecayDescriptor m_decaydescriptor;  /**< Decay descriptor of the decay being reconstructed */
    int m_pdgCode;  /**< PDG code of the combined mother particle */
  };
}
