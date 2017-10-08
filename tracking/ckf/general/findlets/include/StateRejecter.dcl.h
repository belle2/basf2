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

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  template <class AState, class AFilter>
  class StateRejecter : public TrackFindingCDC::Findlet<const AState* const, AState* const> {
  private:
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const AState* const, AState* const>;

  public:
    /// Construct this findlet and add the subfindlet as listener
    StateRejecter();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /**
     * Main function of this findlet: traverse a tree starting from a given seed states.
     *
     * ATTENTION: As described above, the states themselves can be altered during the tree
     * traversal.
     */
    void apply(const std::vector<const AState*>& currentPath,
               const std::vector<AState*>& childStates) final;

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
