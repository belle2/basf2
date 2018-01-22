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

#include <tracking/ckf/general/findlets/OverlapResolver.dcl.h>

#include <tracking/ckf/general/utilities/CKFFunctors.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/numerics/WeightComperator.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParam.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  template<class AFilter>
  OverlapResolver<AFilter>::OverlapResolver() : Super()
  {
    Super::addProcessingSignalListener(&m_filter);
  }

  /// Expose the parameters of the subfindlet
  template<class AFilter>
  void OverlapResolver<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_filter.exposeParameters(moduleParamList, prefix);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "enableOverlapResolving"),
                                  m_param_enableOverlapResolving,
                                  "Enable the overlap resolving.",
                                  m_param_enableOverlapResolving);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useBestNInSeed"),
                                  m_param_useBestNInSeed,
                                  "In seed mode, use only the best seeds.",
                                  m_param_useBestNInSeed);
  }

  template<class AFilter>
  void OverlapResolver<AFilter>::apply(std::vector<typename AFilter::Object>& results,
                                       std::vector<typename AFilter::Object>& filteredResults)
  {
    if (not m_param_enableOverlapResolving or results.empty()) {
      std::swap(results, filteredResults);
      return;
    }

    // Sort results by seed, as it makes the next operations faster
    std::sort(results.begin(), results.end(), TrackFindingCDC::LessOf<SeedGetter>());

    // resolve overlaps in each seed separately
    const auto& groupedBySeed = TrackFindingCDC::adjacent_groupby(results.begin(), results.end(), SeedGetter());
    for (const TrackFindingCDC::VectorRange<Object>& resultsWithSameSeed : groupedBySeed) {

      m_resultsWithWeight.clear();
      for (Object& result : resultsWithSameSeed) {
        TrackFindingCDC::Weight weight = m_filter(result);
        if (std::isnan(weight)) {
          continue;
        }
        m_resultsWithWeight.emplace_back(&result, weight);
      }

      if (not m_resultsWithWeight.empty()) {
        const unsigned int useBestNResults = std::min(m_resultsWithWeight.size(), m_param_useBestNInSeed);
        if (useBestNResults < m_resultsWithWeight.size()) {
          std::sort(m_resultsWithWeight.begin(), m_resultsWithWeight.end(), TrackFindingCDC::GreaterWeight());
        }

        const auto& lastItemToUse = std::next(m_resultsWithWeight.begin(), useBestNResults);
        const auto& longestElement = *(std::max_element(m_resultsWithWeight.begin(), lastItemToUse,
                                                        TrackFindingCDC::LessOf<NumberOfHitsGetter>()));
        filteredResults.push_back(*(longestElement));
      }
    }
  }
}
