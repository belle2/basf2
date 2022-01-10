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
  *This set of module is designed for tracking systematics studies
  *
  */


  /**
   * Tracking efficiency systematics
   */
  class TrackingEfficiencyModule : public Module {
  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TrackingEfficiencyModule();

    /**
    * Function to be executed at each event
    */
    virtual void event() override;

  private:
    /** input particle lists */
    std::vector<std::string> m_ParticleLists;
    /** fraction of particles to be removed from the particlelist */
    double m_frac;
  }; //TrackingEfficiencyModule


  /**
  * Tracking momentum systematics
  */
  class TrackingMomentumModule : public Module {
  public:
    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TrackingMomentumModule();

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
    * Returns the needed scale factor for particle based on tableName and scalingFactorName
    * @param particle
    */
    double getScale(Particle* particle);

    /** input particle lists */
    std::vector<std::string> m_ParticleLists;
    /** input momentum scale modifier */
    double m_scale;

    /** Name of the table  */
    std::string m_tableName;

    /** Name of the table */
    std::string m_scalingFactorName;

    /** Pointer to the table in DB */
    std::unique_ptr<DBObjPtr<ParticleWeightingLookUpTable>> m_ParticleWeightingLookUpTable;
    /**
     * function to set momentum scaling factor
     */
    void setMomentumScalingFactor(Particle* particle);

  }; // TrackingMomentumModule

}; //namespace

