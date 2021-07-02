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
#include <analysis/dataobjects/ParticleList.h>
#include <string>

namespace Belle2 {

  /**
  * Adds Photon Detection Efficiency Data/MC ratios To Particle List
  */
  class PhotonEfficiencySystematicsModule : public Module {

  public:
    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    PhotonEfficiencySystematicsModule();
    /**
    * Get LookUp information for the particle
    * @param particle
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
     * function to add appropriate data/mc ratio weight to a particle
     */
    void addPhotonDetectionEfficiencyRatios(Particle* particle);

  }; // PhotonEfficiencySystematicsModule

}; //namespace
