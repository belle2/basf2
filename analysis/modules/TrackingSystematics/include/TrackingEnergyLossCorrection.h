/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <memory>
#include <string>
#include <vector>

namespace Belle2 {
  template<class T> class DBObjPtr;
  class Particle;
  class ParticleWeightingLookUpTable;

  /**
  * Tracking energy systematics module: allows for the application of an additive energy correction which is read from a ParticleWeightingLookUpTable payload
  */
  class TrackingEnergyLossCorrectionModule final : public Module {
  public:
    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TrackingEnergyLossCorrectionModule();

    /**
    * Initializes the modules and checks the validity of the input parameters
    */
    void initialize() override;

    /**
    * Function to be executed at each beginning of a run
    */
    void beginRun() override;

    /**
    * Function to be executed at each event
    */
    void event() override;

  private:
    /**
    * Returns the needed correction for particle based on payloadName and correctionName
    * @param particle
    */
    double getCorrectionValue(Particle* particle);

    /** input particle lists */
    std::vector<std::string> m_ParticleLists;

    /** input Energy scale modifier */
    double m_correction;

    /** Name of the table  */
    std::string m_payloadName;

    /** Name of the correction from table */
    std::string m_correctionName;

    /** Pointer to the table in DB */
    std::unique_ptr<DBObjPtr<ParticleWeightingLookUpTable>> m_ParticleWeightingLookUpTable;
    /**
     * function to set the Energy correction value
     */
    void setEnergyLossCorrection(Particle* particle);

  }; // TrackingEnergyLossCorrectionModule

}; //namespace

