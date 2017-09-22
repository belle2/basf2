#pragma once

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <algorithm>
#include <cmath>

namespace Belle2 {
  /**
   * Helper function to apply a predicate to a list of states and sort out
   * all states, that have a NAN result of this predicate.
   *
   * If useNResults is > 0, use only the states with the N best results
   * of this predicate.
   *
   * AStateList is expected to be a Range-like object (first, end).
   */
  template <class AStateList, class APredicate>
  void applyAndFilter(AStateList& childStates, APredicate& predicate,
                      unsigned int useNResults = 0)
  {
    if (childStates.empty()) {
      return;
    }

    for (auto* state : childStates) {
      const auto weight = predicate(*state);
      state->setWeight(weight);
    }

    TrackFindingCDC::erase_remove_if(childStates, TrackFindingCDC::IndirectTo<TrackFindingCDC::HasNaNWeight>());

    if (useNResults > 0 and childStates.size() > useNResults) {
      std::sort(childStates.begin(), childStates.end(),
                TrackFindingCDC::LessOf<TrackFindingCDC::IndirectTo<TrackFindingCDC::GetWeight>>());
      childStates.resize(useNResults);
    }
  }
}