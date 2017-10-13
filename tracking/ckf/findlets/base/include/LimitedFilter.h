/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/ckf/utilities/SelectionAlgorithms.h>

namespace Belle2 {
  template <class AFilter>
  class LimitedFilter : public TrackFindingCDC::Findlet<typename AFilter::Object*> {
  private:
    /// The parent class
    using Super = TrackFindingCDC::Findlet<typename AFilter::Object*>;
  public:
    /// Constructor adding the subfindlets as listeners
    LimitedFilter();

    /// Expose the parameters of the filters and our own parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Main function of this findlet: return a range of selected child states for a given current state
    void apply(std::vector<typename AFilter::Object*>& childStates) override;

  private:
    unsigned int m_param_useNResults = 5;

    /// Subfindlet: Filter
    AFilter m_filter;
  };

  template <class AFilter>
  LimitedFilter<AFilter>::LimitedFilter() : Super()
  {
    Super::addProcessingSignalListener(&m_filter);
  }

  template <class AFilter>
  void LimitedFilter<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_filter.exposeParameters(moduleParamList, prefix);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useNResults"), m_param_useNResults,
                                  "Do only use the best N results.", m_param_useNResults);
  }

  template <class AFilter>
  void LimitedFilter<AFilter>::apply(std::vector<typename AFilter::Object*>& childStates)
  {
    applyAndFilter(childStates, m_filter, m_param_useNResults);
  }
}