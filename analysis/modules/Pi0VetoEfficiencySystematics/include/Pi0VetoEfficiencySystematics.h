/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>

#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <string>

namespace Belle2 {

  /**
  * Adds Pi0Veto Efficiency Data/MC ratios To Particle List
  */
  class Pi0VetoEfficiencySystematicsModule : public Module {

  public:
    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    Pi0VetoEfficiencySystematicsModule();
    /**
    * Get LookUp information for the particle
    * @param particle
    * @return LookUp information (map: key - LookUp parameter; value - value of the parameter )
    */
    WeightInfo getInfo(const Particle* particle);

    /** Initializes the module. */
    virtual void initialize() override;

    /**
    * Function to be executed at each event
    */
    virtual void event() override;

  private:
    std::vector<std::string> m_ParticleLists;/**< input particle lists */
    std::string m_decayString;/**< Decay string to select primary photon */
    DecayDescriptor m_decayDescriptor;/**< decay descriptor which specifies the primary photon */
    std::string m_tableName; /**< Table name of the payloads */
    double m_threshold; /**< Threshold of pi0 veto */
    std::string m_mode; /**< Mode of pi0 veto */
    std::string m_suffix; /**< Suffix of extrainfo name */
    std::unique_ptr<DBObjPtr<ParticleWeightingLookUpTable>> m_ParticleWeightingLookUpTable; /**< Pointer to the table in DB */

    /**
     * function to add appropriate data/mc ratio weight to a particle
     */
    void addPi0VetoEfficiencyRatios(Particle* B, const Particle* hardPhoton);

  }; // Pi0VetoEfficiencySystematicsModule

}; //namespace
