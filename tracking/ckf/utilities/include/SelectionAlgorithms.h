#pragma once

#include <cmath>

namespace Belle2 {
  template <class AStateList, class APredicate>
  void applyAndFilter(AStateList& childStates, APredicate& predicate,
                      unsigned int useNResults = 0)
  {
    if (childStates.empty()) {
      return;
    }

    auto firstFiniteState = childStates.begin();
    for (auto& state : childStates) {
      const auto weight = predicate(state);
      if (std::isnan(weight)) {
        std::swap(*firstFiniteState, state);
        firstFiniteState = std::next(firstFiniteState);
      } else {
        state.setWeight(weight);
      }
    }

    childStates.first = firstFiniteState;

    if (useNResults > 0 and childStates.size() > useNResults) {
      std::sort(childStates.begin(), childStates.end());
      childStates.second = std::next(childStates.begin(), useNResults);
    }
  }
}