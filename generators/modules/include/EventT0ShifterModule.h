/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
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
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    StoreArray<MCParticle> m_mcParticles; /**< MC particles */
    StoreObjPtr<MCInitialParticles> m_initialParticles; /**< beam particles */

  };

} // Belle2 namespace

