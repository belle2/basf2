/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dbobjects/ParticleWeightingKeyMap.h>
#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>
#include <string>

namespace Belle2 {

  class ParticleWeightingModule : public Module {

    std::string m_tableName; /**< Name of the table */
    std::string m_inputListName; /**< name of input particle list. */
    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */
    std::unique_ptr<DBObjPtr<ParticleWeightingLookUpTable>> m_ParticleWeightingLookUpTable; /**< Pointer to the table in DB */

  public:

    /**
     * Constructor
     */
    ParticleWeightingModule();

    /**
    * Get LookUp information for the particle
    * @param p particle
    * @return LookUp information (map: key - LookUp parameter; value - value of the parameter )
    */
    WeightInfo getInfo(const Particle* p);

    virtual void initialize() override;
    virtual void event() override;

  };

} // Belle2 namespace

