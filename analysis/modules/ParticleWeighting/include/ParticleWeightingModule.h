/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ParticleWeightingMODULE_H
#define ParticleWeightingMODULE_H

#include <framework/core/Module.h>
#include <analysis/dbobjects/ParticleWeightingKeyMap.h>
#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>
#include <string>

namespace Belle2 {

  class ParticleWeightingModule : public Module {

    std::string m_tableName;
    std::string m_inputListName; /**< name of input particle list. */
    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */
    DBObjPtr<ParticleWeightingLookUpTable> m_ParticleWeightingLookUpTable;
    // // ParticleWeightingLookUpTable* m_ParticleWeightingLookUpTable;

  public:

    /**
     * Constructor
     */
    ParticleWeightingModule();

    // /**
    // * Get LookUp information for the particle
    // * @param p particle
    // * @return LookUp information (map: key - LookUp parameter; value - value of the parameter )
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
