/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIGNALSIDEPARTICLEFILTERMODULE_H
#define SIGNALSIDEPARTICLEFILTERMODULE_H

#include <framework/core/Module.h>
#include <analysis/VariableManager/Utility.h>

#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <memory>

namespace Belle2 {
  /**
   * The module returns true if the current RestOfEvent object is related to any of the Particles in the input ParticleLists.
   *
   * This module should be executed in for_each Rest of Event path only. The ROE objects in DataStore might be created just for Particles
   * from one ParticleList, or they might be created for particles from multiple ParticleLists, or particles reconstructed in multiple
   * decay modes. With this module we can select only those ROE objects that are related to Particles that we are currently interested in.
   * The additional selection criteria is optional.
   */
  class SignalSideParticleFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SignalSideParticleFilterModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void event();


  private:

    std::vector<std::string> m_particleLists;  /**< Name of the input particle lists */

    std::string m_selection;  /**< Additional selection criteria */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

  };
}

#endif /* SIGNALSIDEPARTICLEFILTERMODULE_H */
