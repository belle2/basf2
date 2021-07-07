/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/VariableManager/Utility.h>

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

    /** constructor */
    SignalSideParticleFilterModule();
    /** initialize the module (setup the data store) */
    virtual void initialize() override;
    /** process event */
    virtual void event() override;


  private:

    std::vector<std::string> m_particleLists;  /**< Name of the input particle lists */
    std::string m_selection;  /**< Additional selection criteria */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

  };
}

