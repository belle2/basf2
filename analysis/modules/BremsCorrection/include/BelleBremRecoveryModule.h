/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor : Soumen Halder and Saurabh Sandilya                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <string>


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
    StoreObjPtr<ParticleList>  m_inputparticleList; /**<StoreObjptr for input charged particlelist */
    StoreObjPtr<ParticleList>  m_gammaList; /**<StoreObjptr for gamma list */
    StoreObjPtr<ParticleList>  m_outputparticleList; /**<StoreObjptr for output particlelist */
    StoreObjPtr<ParticleList>  m_outputAntiparticleList; /**<StoreObjptr for output antiparticlelist */
    StoreArray<Particle> m_particles; /**< StoreArray of Particle objects */
    StoreArray<MCParticle> m_mcParticles; /**< StoreArray of MCParticle objects */
    StoreArray<PIDLikelihood> m_pidlikelihoods; /**< StoreArray of PIDLikelihood objects */
    double m_angleThres; /**< input max angle to be accepted (in radian) */
    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */
    bool m_isMultiPho; /**<multiple or one bremphoton addition option  */
  };

} // Belle2 namespace

