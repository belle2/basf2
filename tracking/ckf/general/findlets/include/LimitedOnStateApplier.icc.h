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

#include <tracking/ckf/general/findlets/LimitedOnStateApplier.dcl.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/numerics/WeightComperator.h>
#include <tracking/ckf/general/findlets/OnStateApplier.icc.h>

#include <framework/core/ModuleParamList.icc.h>

#include <algorithm>

namespace Belle2 {
  template <class AState, class AFilter>
  LimitedOnStateApplier<AState, AFilter>::LimitedOnStateApplier()
  {
    this->addProcessingSignalListener(&m_filter);
  };

  template <class AState, class AFilter>
  void LimitedOnStateApplier<AState, AFilter>::apply(const std::vector<TrackFindingCDC::WithWeight<const AState*>>& currentPath,
                                                     std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates)
  {
    Super::apply(currentPath, childStates);

    if (m_param_useNStates > 0 and childStates.size() > static_cast<unsigned int>(m_param_useNStates)) {
      std::sort(childStates.begin(), childStates.end(), TrackFindingCDC::LessOf<TrackFindingCDC::GetWeight>());
      childStates.erase(childStates.begin() + m_param_useNStates, childStates.end());
    }
  };

  template <class AState, class AFilter>
  TrackFindingCDC::Weight LimitedOnStateApplier<AState, AFilter>::operator()(const Object& object)
  {
    return m_filter(object);
  };

  template <class AState, class AFilter>
  void LimitedOnStateApplier<AState, AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_filter.exposeParameters(moduleParamList, prefix);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useNStates"), m_param_useNStates, "Only use the best N states",
                                  m_param_useNStates);
  };
}
