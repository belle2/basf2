/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Utility.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/core/Module.h>

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <memory>

namespace Belle2 {

  /**
   * Loops over all Particles in the ParticleList and removes those Particles
   * from the ParticleList that do not pass given selection criteria.
   */
  class ParticleSelectorModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleSelectorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    StoreObjPtr<ParticleList> m_particleList; /**< particle list */

    std::string m_decayString;   /**< Input DecayString specifying the particle being selected */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the particle being selected */

    std::string m_listName;   /**< output particle list name */

    std::string m_cutParameter;  /**< selection criteria */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */
  };

} // Belle2 namespace

