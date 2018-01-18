/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {
  /// Module which sets its return value based on the fact, if the event is in the given run or not.
  class RunDependentConditionModule : public Module {
  public:
    /// Add the module parameters and the description.
    RunDependentConditionModule();

    /// Require the event meta data
    void initialize() override;

    /// Sets the value of m_runConditionMet according to the vale in the event meta data.
    void beginRun() override;

    /// Returns the value of m_runConditionMet.
    void event() override;

  private:
    /// Returns true if in this run - or false otherwise.
    int m_trueOnRun;

    /// Storage for the event meta data
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /// Internal boolean if the condition to be in run m_trueOnRun is true or false. Will only be set in beginRun()
    bool m_runConditionMet = false;
  };

}
