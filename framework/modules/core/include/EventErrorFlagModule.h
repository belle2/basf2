#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  /** Module to check or add error flags of the EventMetaData.
   *
   * This module returns the error flags of the EventMetaData.
   * It can add further error flags.
   */
  class EventErrorFlagModule : public Module {
  public:
    /** Constructor of the module. Sets the description of the module and the parameters of the module. */
    EventErrorFlagModule();

    /** Initializes the datastore pointers */
    virtual void initialize() override;

    /** Checks and adds the EventMetaData error flags */
    virtual void event() override;

  private:
    /** Error flags to add */
    int m_ErrorFlag;

    /** event meta data */
    StoreObjPtr<EventMetaData> m_eventMetaData;
  };
}
