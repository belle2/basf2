/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>
#include <hlt/dataobjects/OnlineEventT0.h>


namespace Belle2 {

  /** Module to write the EventT0s computed on HLT
   */
  class OnlineEventT0CreatorModule : public Module {

  public:

    /** Default constructor */
    OnlineEventT0CreatorModule();

    /** Destructor */
    virtual ~OnlineEventT0CreatorModule() = default;

    /** initialize: check DataStore content*/
    void initialize() override;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    void event() override;

    /* StoreArray of OnlineEventT0*/
    StoreArray<OnlineEventT0> m_onlineEventT0;

    /* StoreArray of EventT0*/
    StoreObjPtr<EventT0> m_eventT0;

  private:

  };
} // Belle2 namespace
