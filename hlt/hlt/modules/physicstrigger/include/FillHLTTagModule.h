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
#include "mdst/dataobjects/HLTTag.h"
#include <hlt/hlt/dataobjects/PhysicsTriggerInformation.h>

#include <string>
#include <memory>

namespace Belle2 {
  /**
   * physics trigger
   *
   *    *
   */
  class FillHLTTagModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    FillHLTTagModule();

    /** Destructor */
    virtual ~FillHLTTagModule();

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

    /**Fill the trigger info,*/
    bool eventSelect();

  private:
    StoreArray<HLTTag> m_hltTag; /** Array for HLTTag */
    StoreArray<PhysicsTriggerInformation> m_physicsTriggerInformation; /** Array for PhysicsTriggerInformation */



  };
}
