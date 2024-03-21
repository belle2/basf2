/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>

#include <generators/modules/GeneratorBaseModule.h>
#include <generators/aafh/AAFHInterface.h>
#include <generators/utilities/InitialParticleGeneration.h>

#include <string>

namespace Belle2 {
  /**
   * AAFH Generator to generate 2-fermion events like e+e- -> e+e-e+e-
   */
  class AafhInputModule : public GeneratorBaseModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    AafhInputModule();

    /** initialize generator */
    virtual void generatorInitialize() override;

    /** generate event */
    virtual void generatorEvent() override;

    /** calculate cross section */
    virtual void terminate() override;

  private:
    /** Generator mode */
    int m_mode;

    /** Rejection mode */
    int m_rejection;

    /** maximum number of tries for generating an event */
    int m_maxTries;

    /** maximum expected final weight for rejection scheme */
    double m_maxFinalWeight;

    /** maximum expected subgenerator weight for rejection scheme */
    double m_maxSubgeneratorWeight;

    /** minimum invariant mass of generated fermion pair */
    double m_minMass;

    /** name of the generated particle for modes c_MuonParticle and c_ElectronParticle */
    std::string m_particle;

    /** relative subgenerator weights */
    std::vector<double> m_subgeneratorWeights;

    /** suppression limits for the matrix element calculations */
    std::vector<double> m_suppressionLimits;

    /** MCParticle collection */
    StoreArray<MCParticle> m_mcparticles;

    /** interface to the generator */
    AAFHInterface m_generator;

    /** Method is called to initialize the generator. */
    void initializeGenerator();

    bool m_initialized{false}; /**< True if generator has been initialized. */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    /** initial particle used by BeamParameter class */
    InitialParticleGeneration m_initial;
  };
}

