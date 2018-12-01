/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <analysis/DecayDescriptor/DecayDescriptor.h>


// DataStore
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// DataObjects
#include <framework/dbobjects/BeamParameters.h>
#include <analysis/dataobjects/ParticleList.h>


namespace Belle2 {
  /**
   * Redefine (select) the daughters of a particle
   *
   * useful for recoil analysis   *
   */
  class SelectDaughtersModule : public Module {

  public:
    /** constructor */
    SelectDaughtersModule();
    /** destructor */
    ~SelectDaughtersModule() {}
    /** set up datastore */
    virtual void initialize() override;
    /** process the event */
    virtual void event() override;
  private:
    std::string m_listName;  /**< name of particle list */
    std::string m_decayString;    /**< daughter particles selection */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of decays to look for. */
  };
}
