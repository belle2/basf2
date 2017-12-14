/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <hlt/hlt/dataobjects/PhysicsTriggerInformation.h>

#include <string>
#include <memory>

namespace Belle2 {
  /**
   * physics trigger
   *
   *    *
   */
  class PhysicsTriggerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PhysicsTriggerModule();

    /** Destructor */
    virtual ~PhysicsTriggerModule();

    /** Initialize the parameters */
    virtual void initialize();

    /** Called when entering a new run. */
    virtual void beginRun();

    /** Event processor. */
    virtual void event();

    /** End-of-run action. */
    virtual void endRun();

    /** Termination action. */
    virtual void terminate();


    /**Perform the selection criteria on event*/
    bool eventSelect();

    /**
      Collect the information of event from VariableManager,
      and store them in StoreArray<PhysicsTriggerInformation>
     */
    void eventInformation(PhysicsTriggerInformation& phy);

  private:
    StoreArray<PhysicsTriggerInformation> m_physicsTriggerInformation; /** Array for PhysicsTriggerInformation */


    /**Access users' cut */
    std::string m_hltcut;

    /**Access users' cut*/
    std::unique_ptr<Variable::Cut> m_cut;

    /**the trigger categories*/
    int m_trgcategory;

  };
}
