/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GENERATORS_MODULES_AAFHINPUT_AAFHINPUTMODULE_H
#define GENERATORS_MODULES_AAFHINPUT_AAFHINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <generators/aafh/AAFHInterface.h>

#include <string>
#include <TLorentzRotation.h>

namespace Belle2 {
  /**
   * AAFH Generator to generate 2-fermion events like e+e- -> e+e-e+e-
   */
  class AafhInputModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    AafhInputModule();

    /** initialize generator */
    virtual void initialize();

    /** generate event */
    virtual void event();

    /** calculate cross section */
    virtual void terminate();

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
    /** Lorentz transformation to go from CMS to lab after generation */
    TLorentzRotation m_labBoost;
  };
}

#endif /* GENERATORS_MODULES_AAFHINPUT_AAFHINPUTMODULE_H */
