/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nataliia Kovalchuk and Savino Longo                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>

#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <string>
namespace Belle2 {

  /**
  * Energy bias correction
  */
  class EnergyBiasCorrectionModule : public Module {

  public:
    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    EnergyBiasCorrectionModule();
    /**
    * Get LookUp information for the particle
    * @param p particle
    * @return LookUp information (map: key - LookUp parameter; value - value of the parameter )
    */
    WeightInfo getInfo(const Particle* particle);

    /** Nothing so far.*/
    virtual void beginRun() override;
    /**
    * Function to be executed at each event
    */
    virtual void event() override;

  private:
    /** input particle lists */
    std::vector<std::string> m_ParticleLists;
    std::string m_tableName; /**< Name of the table */
    std::unique_ptr<DBObjPtr<ParticleWeightingLookUpTable>> m_ParticleWeightingLookUpTable; /**< Pointer to the table in DB */

    /**
     * function to set scaling factor
     */
    void setEnergyScalingFactor(Particle* particle);

  }; // EnergyBiasCorrectionModule

}; //namespace
