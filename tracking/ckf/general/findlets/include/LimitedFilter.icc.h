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

#include <tracking/ckf/general/findlets/LimitedFilter.dcl.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/numerics/WeightComperator.h>

namespace Belle2 {
  template <class AState, class AFilter>
  LimitedFilter<AState, AFilter>::LimitedFilter() : Super()
  {
    Super::addProcessingSignalListener(&m_filter);
  };

  /// Expose the parameters of the subfindlet
  template <class AState, class AFilter>
  void LimitedFilter<AState, AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_filter.exposeParameters(moduleParamList, prefix);
  };

  template <class AState, class AFilter>
  void LimitedFilter<AState, AFilter>::apply(const std::vector<const AState*>& currentPath,
                                             std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates)
  {
    if (childStates.empty()) {
      return;
    }

    for (TrackFindingCDC::WithWeight<AState*>& state : childStates) {
      const auto weight = m_filter(std::make_pair(currentPath, state));
      state.setWeight(weight);
    }

    TrackFindingCDC::erase_remove_if(childStates, TrackFindingCDC::HasNaNWeight());

    if (m_param_useNStates > 0 and childStates.size() > m_param_useNStates) {
      std::sort(childStates.begin(), childStates.end(), TrackFindingCDC::LessOf<TrackFindingCDC::GetWeight>());
      childStates.erase(childStates.begin() + m_param_useNStates, childStates.end());
    }
  };
}
