/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>


namespace Belle2 {

  /** Module to combine the EventT0 values from multiple sub-detectors
   */
  class EventT0CombinerModule final : public Module {

  public:

    /** Default constructor */
    EventT0CombinerModule();

    /** Destructor */
    virtual ~EventT0CombinerModule() = default;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    void event() override;

  private:

    /// computes the new average between multiple, un-correlated sub-detector measurements
    EventT0::EventT0Component computeCombination(std::vector<EventT0::EventT0Component> measurements) const;

    /// Access to global EventT0 which will be read and updated
    StoreObjPtr<EventT0> m_eventT0;

  };
} // Belle2 namespace
