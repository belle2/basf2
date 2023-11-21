/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>

#include <analysis/dataobjects/Particle.h>

namespace Belle2 {

  /**
  * Tracking momentum systematics module: allows for the application of a corrective momentum scale factor which is read from a ParticleWeightingLookUpTable payload
  */
  class TrackingMomentumScaleFactorsModule : public Module {
  public:
    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TrackingMomentumScaleFactorsModule();

    /**
    * Initializes the modules and checks the validity of the input parameters
    */
    virtual void initialize() override;

    /**
    * Function to be executed at each event
    */
    virtual void event() override;

  private:
    /**
    * Returns the needed scale factor for particle based on payloadName and scalingFactorName
    * @param particle
    */
    double getScalingFactor(Particle* particle);

    /**
    * Returns the needed smearing factor for particle based on payloadName and smearingFactorName
    * @param particle
    */
    double getSmearingFactor(Particle* particle);

    /** input particle lists */
    std::vector<std::string> m_ParticleLists;
    /** input momentum scale modifier */
    double m_scale;

    /** Name of the table  */
    std::string m_payloadName;

    /** Name of the scale factor from table */
    std::string m_scalingFactorName;

    /** Name of the smear factor from table */
    std::string m_smearingFactorName;

    /** Pointer to the table in DB */
    std::unique_ptr<DBObjPtr<ParticleWeightingLookUpTable>> m_ParticleWeightingLookUpTable;
    /**
     * function to set momentum scaling factor
     */
    void setMomentumScalingFactor(Particle* particle);

    /**
     * function to set momentum scaling factor
     */
    void setMomentumSmearingFactor(Particle* particle);

  }; // TrackingMomentumScaleFactorsModule

}; //namespace

