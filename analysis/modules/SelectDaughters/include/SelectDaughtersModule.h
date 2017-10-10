/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SELECTDAUGHTERSMODULE_H
#define SELECTDAUGHTERSMODULE_H

#include <framework/core/Module.h>
#include <string>
//#include <vector>
//#include <tuple>
//#include <memory>
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

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SelectDaughtersModule();

    /** sdf */
    virtual ~SelectDaughtersModule();

    /** sdf */
    virtual void initialize();

    /** sdf */
    virtual void beginRun();

    /** sdf */
    virtual void event();

    /** sdfy */
    virtual void endRun();

    /** sdf */
    virtual void terminate();


  private:

    std::string m_listName;  /**< name of particle list */
    std::string m_decayString;    /**< daughter particles selection */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of decays to look for. */
  };
}

#endif /* SELECTDAUGHTERSMODULE_H */
