#pragma once

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
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

    using State = typename AStateList::value_type;
    const auto& weightIsNan = [](const State & state) {
      return std::isnan(state->getWeight());
    };

    TrackFindingCDC::erase_remove_if(childStates, weightIsNan);

    if (useNResults > 0 and childStates.size() > useNResults) {
      std::sort(childStates.begin(), childStates.end(), TrackFindingCDC::LessOf<TrackFindingCDC::Deref>());


      const auto& firstTruth = std::find_if(childStates.begin(), childStates.end(), [](const auto * state) {
        return state->getTruthInformation();
      });
      if (firstTruth != childStates.end()) {
        const auto& indexOfFirstTruth = std::distance(childStates.begin(), firstTruth);
        if (indexOfFirstTruth > useNResults) {
          B2WARNING("First truth is in " << indexOfFirstTruth << " of " << childStates.size() << " where limit is "
                    << useNResults);
        }
      }
      childStates.resize(useNResults);
    }
  }
}