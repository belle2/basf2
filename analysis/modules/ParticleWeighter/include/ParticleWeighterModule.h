/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ParticleWeighterMODULE_H
#define ParticleWeighterMODULE_H

#include <framework/core/Module.h>
#include <analysis/dbobjects/KeyMap.h>
#include <analysis/dbobjects/LookupTable.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>
#include <string>

namespace Belle2 {

  class ParticleWeighterModule : public Module {

    std::string m_tableName;
    std::string m_inputListName; /**< name of input particle list. */
    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */
    DBObjPtr<LookupTable> m_LookupTable;
    // // LookupTable* m_LookupTable;

  public:

    /**
     * Constructor
     */
    ParticleWeighterModule();

    // /**
    // * Get lookup information for the particle
    // * @param p particle
    // * @return lookup information (map: key - lookup parameter; value - value of the parameter )
    // */
    // WeightInfo getInfo(const Particle* p);

    // ///**
    // // * Get key ID for given particle. If particle is out of range, -1 is returned.
    // // * @param particle
    // // * @return key ID
    // // */
    // double getKey(const Particle* p);

    virtual void initialize() override;
    virtual void event() override;

  };

} // Belle2 namespace

#endif
