/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/OverlapResolver.dcl.h>

#include <tracking/ckf/general/utilities/CKFFunctors.h>

#include <tracking/trackingUtilities/utilities/Functional.h>
#include <tracking/trackingUtilities/numerics/WeightComperator.h>
#include <tracking/trackingUtilities/utilities/Algorithms.h>
#include <tracking/trackingUtilities/utilities/VectorRange.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>

#include <framework/core/ModuleParam.h>
#include <framework/logging/Logger.h>

#include <type_traits>
#include <utility>

namespace Belle2 {

  namespace overlapResolverDetail {
    /**
     * Type trait detecting whether a type provides a callable const getArrayIndex() member.
     * This is the primary template, selected when T has no getArrayIndex() member.
     */
    template<class T, class = void>
    struct HasGetArrayIndex : std::false_type {};

    /**
     * Specialization selected when T exposes a const getArrayIndex() member.
     */
    template<class T>
struct HasGetArrayIndex<T, std::void_t<decltype(std::declval<const T&>().getArrayIndex())>> : std::true_type {};
  }

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

    moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "enableOverlapResolving"),
                                  m_param_enableOverlapResolving,
                                  "Enable the overlap resolving.",
                                  m_param_enableOverlapResolving);
    moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "useBestNInSeed"),
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

    // The seed ordering below relies on getSeed()->getArrayIndex(); require it explicitly
    // so a future instantiation with an unsuitable seed type fails with a clear message.
    using SeedType = std::remove_cv_t<std::remove_pointer_t<
                     decltype(std::declval<typename AFilter::Object>().getSeed())>>;
    static_assert(overlapResolverDetail::HasGetArrayIndex<SeedType>::value,
                  "OverlapResolver orders seed groups by getSeed()->getArrayIndex() to keep the "
                  "order reproducible; the result seed type must provide a getArrayIndex() method.");

    // Sort results by seed, as it makes the next operations faster.
    // Order by the seed's array index rather than its heap address.
    // Note that a seed here is a RecoTrack* from a StoreArray, so getArrayIndex() is well defined.
    std::stable_sort(results.begin(), results.end(),
    [](const typename AFilter::Object & lhs, const typename AFilter::Object & rhs) {
      return lhs.getSeed()->getArrayIndex() < rhs.getSeed()->getArrayIndex();
    });

    // resolve overlaps in each seed separately
    const auto& groupedBySeed = TrackingUtilities::adjacent_groupby(results.begin(), results.end(), SeedGetter());
    for (const TrackingUtilities::VectorRange<Object>& resultsWithSameSeed : groupedBySeed) {

      m_resultsWithWeight.clear();
      for (Object& result : resultsWithSameSeed) {
        TrackingUtilities::Weight weight = m_filter(result);
        if (std::isnan(weight)) {
          continue;
        }
        m_resultsWithWeight.emplace_back(&result, weight);
      }

      if (not m_resultsWithWeight.empty()) {
        // sort results so that 'std::max' below picks path with highest weight if multiple paths have same size
        std::sort(m_resultsWithWeight.begin(), m_resultsWithWeight.end(), TrackingUtilities::GreaterWeight());

        const unsigned int useBestNResults = std::min(m_resultsWithWeight.size(), m_param_useBestNInSeed);
        const auto& lastItemToUse = std::next(m_resultsWithWeight.begin(), useBestNResults);
        const auto& longestElement = *(std::max_element(m_resultsWithWeight.begin(), lastItemToUse,
                                                        TrackingUtilities::LessOf<NumberOfHitsGetter>()));
        filteredResults.push_back(*(longestElement));
      }
    }
  }
}
