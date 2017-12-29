/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  template <class AState, class AFilter>
  class StateRejecter : public
    TrackFindingCDC::Findlet<const TrackFindingCDC::WithWeight<const AState*>, TrackFindingCDC::WithWeight<AState*>> {
  private:
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const TrackFindingCDC::WithWeight<const AState*>, TrackFindingCDC::WithWeight<AState*>>;

  public:
    /// Construct this findlet and add the subfindlet as listener
    StateRejecter();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /**
     */
    void apply(const std::vector<TrackFindingCDC::WithWeight<const AState*>>& currentPath,
               std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates) final;

  private:
    /// State filter to decide which available continuations should be traversed next.
    AFilter m_firstFilter;
    /// State filter to advance all states.
    AFilter m_advanceFilter;
    /// State filter to decide which available continuations should be traversed next.
    AFilter m_secondFilter;
    /// State filter to kalman update all states.
    AFilter m_updateFilter;
    /// State filter to decide which available continuations should be traversed next.
    AFilter m_thirdFilter;
  };
}
