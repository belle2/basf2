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
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {
  class EventLevelTriggerTimeInfo;

  /** Module to combine the EventT0 values from multiple sub-detectors
   */
  class EventT0CombinerModule final : public Module {

  public:

    /** Default constructor */
    EventT0CombinerModule();

    /** Destructor */
    virtual ~EventT0CombinerModule() = default;

    /** Initialize the module */
    void initialize() override;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    void event() override;

  private:

    /// computes the new average between multiple, un-correlated sub-detector measurements
    EventT0::EventT0Component computeCombination(std::vector<EventT0::EventT0Component> measurements) const;

    /// Access to global EventT0 which will be read and updated
    StoreObjPtr<EventT0> m_eventT0;

    /// Access to EventLevelTriggerTimeInfo object
    StoreObjPtr<EventLevelTriggerTimeInfo> m_eventLevelTriggerTimeInfo;

  };
} // Belle2 namespace
