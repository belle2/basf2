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
  class LimitedFilter : public TrackFindingCDC::Findlet<const AState* const, TrackFindingCDC::WithWeight<AState*>> {
  private:
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const AState* const, TrackFindingCDC::WithWeight<AState*>>;

  public:
    /// Construct this findlet and add the subfindlet as listener
    LimitedFilter();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /**
     */
    void apply(const std::vector<const AState*>& currentPath,
               std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates) final;

  private:
    /// State filter to decide which available continuations should be traversed next.
    AFilter m_filter;

    /// Parameter how many objects should pass maximal
    int m_param_useNStates = 0;
  };
}
