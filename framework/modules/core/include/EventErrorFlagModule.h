#pragma once
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
