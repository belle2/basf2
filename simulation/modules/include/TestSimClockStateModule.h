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
#include <simulation/dataobjects/SimClockState.h>

#include <vector>

namespace Belle2 {
  /**
   * Module to set the phases between the clocks of the different sub-detectors
   *
   * This module generates a random phase that is than saved in datastore.
   *
   * @sa EventMetaData
   */
  class TestSimClockStateModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    TestSimClockStateModule();

    /** Destructor. */
    virtual ~TestSimClockStateModule();

    /** Initializes the Module.
     *
     * Does a check of the parameters and gives an error of the parameters
     * were not correctly set by the user.
     */
    virtual void initialize() override;


    /** Stores the phases into the DataStore.
     *
     * Based on the random numbers and ec...
     *
     */
    virtual void event() override;


  protected:


  private:

    StoreObjPtr<SimClockState> m_clockState; /**< Input object. */

  };
}
