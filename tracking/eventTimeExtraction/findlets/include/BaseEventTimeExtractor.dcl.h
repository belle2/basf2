/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {
  template<class ... AIOTypes>
  class BaseEventTimeExtractor : public TrackFindingCDC::Findlet<AIOTypes...> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<AIOTypes...>;

  public:
    /// Returns true if the last run t0 extraction was successful
    bool wasSuccessful() const;

    /// Expose our parameters to the super module
    virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Initialize the event t0 store obj ptr.
    virtual void initialize() override;

    /// Create the event t0 store obj ptr.
    virtual void beginEvent() override;

  protected:
    void resetEventT0() const;

    /// Pointer to the storage of the eventwise T0 estimation in the data store.
    StoreObjPtr<EventT0> m_eventT0;

    /// Variable to show that the execution was successful
    bool m_wasSuccessful = false;

  private:
    /// Parameter if it is fine to overwrite the current EventT0
    bool m_param_overwriteExistingEstimation = true;

    /// Storage for the event t0 before this module ran
    boost::optional<EventT0::EventT0Component> m_eventT0Before;
  };
}
