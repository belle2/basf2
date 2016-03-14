/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GENERATORPRESELECTIONMODULE_H
#define GENERATORPRESELECTIONMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>
#include <vector>

namespace Belle2 {
  /**
   * generator preselection
   *
   *    *
   */
  class GeneratorPreselectionModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    GeneratorPreselectionModule();

    /** Destructor */
    virtual ~GeneratorPreselectionModule();

    /** Initialize the parameters */
    virtual void initialize();

    /** Event processor. */
    virtual void event();

    /** Termination action. */
    virtual void terminate();

  protected:
    /** called for each particle, checks for cuts. */
    void checkParticle(const MCParticle& mc, int level = 0);

    std::string m_particleList; /**< The name of the MCParticle collection. */
    std::vector<bool> m_seen;   /**< Tag the particles which were already visited using their index. */
    bool m_onlyPrimaries;       /**< Print only primary particles. */
    int m_maxLevel;             /**< Show only up to specified depth level. */
    StoreArray<MCParticle> m_mcparticles; /**< store array for the MCParticles */

  private:

    /**charged*/
    int m_nCharged; /**< number of charged particles (counted per event). */
    int m_nChargedMin; /**< minimum number of charged particles. */
    int m_nChargedMax; /**< minimum number of charged particles. */

    /**selection criteria for charged*/
    double m_MinChargedP; /**< minimum p for each charged particle. */
    double m_MinChargedPt; /**< minimum pT for each charged particle. */
    double m_MinChargedTheta; /**< minimum theta for each charged particle. */
    double m_MaxChargedTheta; /**< maximum theta for each charged particle. */

    /**number of photons*/
    double m_nPhoton; /**< number of photons (counted per event). */
    int m_nPhotonMin; /**< minimum number of photons. */
    int m_nPhotonMax; /**< minimum number of photons. */

    /**selection criteria for photons*/
    double m_MinPhotonEnergy; /**< minimum energy for each photon. */
    double m_MinPhotonTheta; /**< minimum theta for each photon. */
    double m_MaxPhotonTheta; /**< maximum theta for each photon. */

  };
}

#endif /* GENERATORPRESELECTIONMODULE_H */
