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
#include <framework/database/IntervalOfValidity.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <vector>

namespace Belle2 {
  /// Module which sets its return value based on the fact, if the event is in the given run/exp interval or not
  class IoVDependentConditionModule : public Module {
  public:
    /// Add the module parameters and the description.
    IoVDependentConditionModule();

    /// Require the event meta data and turn the minimal/maximal exp/runs to an IoV.
    void initialize() override;

    /// Set the m_conditionIsMet according to the new run
    void beginRun() override;

    /// Returns true, if the event is in the given IoV.
    void event() override;

  private:
    /// Returns true if in this IoV
    std::vector<IntervalOfValidity> m_iovsToCheck;

    /// Internal condition: true if run/exp is in IoV
    bool m_conditionIsMet = false;

    /// Storage for the event meta data
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /// Parameter for the input iov list
    std::vector<std::tuple<int, int, int, int>> m_iovList;
  };

}
