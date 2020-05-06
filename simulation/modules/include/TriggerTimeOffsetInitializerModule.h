#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <simulation/dataobjects/TriggerTimeOffset.h>

#include <vector>

namespace Belle2 {
  /**
   * Module to set the offset between the trigger signal and the clocks of the different sub-detectors
   *
   * This module generates a random offset that is than saved in datastore.
   *
   */
  class TriggerTimeOffsetInitializerModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */

    TriggerTimeOffsetInitializerModule();

    /** Destructor. */
    virtual ~TriggerTimeOffsetInitializerModule();

    /** Initializes the Module.
     *
     */
    virtual void initialize() override;


    /** Stores the offset into the DataStore.
     *
     *
     */
    virtual void event() override;

  protected:

  private:

    StoreObjPtr<TriggerTimeOffset> m_trgOffSetPtr; /**< Output object. */
    Float_t m_seed;
  };
}
