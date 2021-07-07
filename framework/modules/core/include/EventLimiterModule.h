/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {
  /** The EventLimiter module.
   *
   * This module returns true on each event until reaching the maximum number of events in a run.
   * Can set the value of the maximum via an input parameter.
   * This module can be used with a conditional basf2 path to control the number of events passing this module.
   */
  class EventLimiterModule : public Module {

  public:
    /** Constructor: Sets the description, the properties and the parameters of the module. */
    EventLimiterModule();

    /** Destructor of the module */
    virtual ~EventLimiterModule() {};

    /** Initialization states required data objects (EventMetaData) */
    virtual void initialize() override;

    /** Checks if we've reached the maximum number of events yet and sets the return value to False if we have */
    virtual void event() override;

    /** Resets our event counter to zero and return value to True */
    virtual void beginRun() override;

  private:
    /** Datastore pointers */
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /** Maximum number of events to be collected at the start of each run (-1 = no maximum) */
    int m_maxEventsPerRun;

    /** Flag that will be returned by the module. Gets set to True at the start of the run
     * and False once we hit the maximum events */
    bool m_returnValue = true;

    /** How many events processed for the current run so far, stops counting up once max is hit
     * Only used/incremented if m_maxEventsPerRun > -1 */
    int m_runEvents = 0;
  };
}
