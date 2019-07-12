/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor : Soumen Halder and Saurabh Sandilya                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <string>
//#include <vector>
//#include <tuple>
//#include <memory>


namespace Belle2 {
  class Particle;

  /**
   * Brem recovery module (used in past belle analyses)
   * This module add four vector of all the brem photon to the four vector associated to the charged particle
   *
   */
  class BelleBremRecoveryModule : public Module {

  public:

    /**
     * Constructor
     */
    BelleBremRecoveryModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    enum {c_DimMatrix = 7};

  private:

    int m_pdgCode;                /**< PDG code of the combined mother particle */

    std::string m_decayString;   /**< Input DecayString specifying the decay being reconstructed */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the charged particle decay */
    DecayDescriptor m_decaydescriptorGamma; /**< Decay descriptor of the decay being reconstructed */
    std::string m_inputListName; /**< input ParticleList names */
    std::string m_gammaListName; /**< input ParticleList names */
    std::string m_outputListName; /**< output ParticleList name */
    std::string m_outputAntiListName;   /**< output anti-particle list name */
    double m_angleThres; /**< input max angle to be accepted (in radian) */
    double m_energy_min; /**< min energy of gamma to be accepted(in GeV) */
    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

  };

} // Belle2 namespace

