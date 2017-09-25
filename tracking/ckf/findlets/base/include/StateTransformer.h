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

#include <vector>

namespace Belle2 {
  /**
   * Class to transform a vector range of hits into states.
   *
   * As we do not want to recreate the states again and again, we use a cache for the states here
   * and reuse them. Only pointer to those states will be returned.
   *
   * As we have to keep those states in memory until all of their children are processed, we keep
   * a separate cache for every layer.
   */
  template<class AState, unsigned int N>
  class StateTransformer {
  public:
    /// The class of the hit
    using HitPtr = const typename AState::HitObject*;

    /// Use the hits and transform them into states, by reusing an internal cache of states
    template<class AHitArray>
    void transform(const AHitArray& matchingHits, std::vector<AState*>& childStates,
                   AState* currentState);

  private:
    /// Temporary object pool for finding the next state
    std::array < std::vector<AState>, N + 1 > m_temporaryStates;
  };

  template <class AState, unsigned int N>
  template <class AHitArray>
  void StateTransformer<AState, N>::transform(const AHitArray& matchingHits, std::vector<AState*>& childStates,
                                              AState* currentState)
  {
    // As we do not want to recreate the states again and again, we reuse temporary states here
    // Because those have to exist until each of them is processed, we use one vector of temporary states for each layer
    auto& temporaryStates = m_temporaryStates[currentState->getNumber()];

    if (matchingHits.size() + 1 > temporaryStates.size()) {
      temporaryStates.resize(matchingHits.size() + 1);
    }

    // reuse the former temporary states, but reset them to the new hits
    auto lastState = temporaryStates.begin();
    for (const auto& hit : matchingHits) {
      lastState->set(currentState, hit);
      lastState = std::next(lastState);
    }

    // Also add an empty state to make layer-jumping possible
    lastState->set(currentState, nullptr);
    lastState = std::next(lastState);

    // The next findlets should only work with pointer to those objects, because they are smaller and easier to
    // handle. So we feed them a vector with pointers, pointing to our temporary state objects
    childStates.reserve(matchingHits.size() + 1);

    for (auto iterator = temporaryStates.begin(); iterator != lastState; ++iterator) {
      AState& childState = *iterator;
      childStates.push_back(&childState);
    }
  }
}