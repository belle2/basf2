/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/dataobjects/MCInitialParticles.h>


namespace Belle2 {

  /**
   * Module that shifts the time of all MCParticles
   * so that the collision is at t=0.
   */
  class EventT0ShifterModule : public Module {

  public:

    /**
     * Constructor
     */
    EventT0ShifterModule();

    /**
     * Initialize the Module.
     */
    virtual void initialize();

    /**
     * Event processor.
     */
    virtual void event();

  private:

    StoreArray<MCParticle> m_mcParticles; /**< MC particles */
    StoreObjPtr<MCInitialParticles> m_initialParticles; /**< beam particles */

  };

} // Belle2 namespace

