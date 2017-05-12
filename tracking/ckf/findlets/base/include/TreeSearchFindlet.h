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
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  template<class AStateObject, class AHitSelector>
  class TreeSearchFindlet : public TrackFindingCDC::Findlet <
    typename AStateObject::SeedObject*,
    const typename AStateObject::HitObject*,
    std::pair<typename AStateObject::SeedObject*, std::vector<const typename AStateObject::HitObject*>>
        > {
  public:
    using SeedPtr = typename AStateObject::SeedObject*;
    using HitPtr = const typename AStateObject::HitObject*;
    using StateArray = typename std::array < AStateObject, AStateObject::N + 1 >;
    using StateIterator = typename StateArray::iterator;
    using ResultPair = std::pair<typename AStateObject::SeedObject*, std::vector<const typename AStateObject::HitObject*>>;

    using Super = TrackFindingCDC::Findlet<SeedPtr, HitPtr, ResultPair>;

    /// Construct this findlet and add the subfindlet as listener
    TreeSearchFindlet() : Super()
    {
      Super::addProcessingSignalListener(&m_hitSelector);
    }

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
      m_hitSelector.exposeParameters(moduleParamList, prefix);
    }

    /// Main function of this findlet: traverse a tree starting from a given seed object.
    void apply(std::vector<SeedPtr>& seedsVector, std::vector<HitPtr>& hitVector,
               std::vector<ResultPair>& results) override
    {
      for (SeedPtr seed : seedsVector) {
        StateIterator firstStateIterator = m_states.begin();
        firstStateIterator->initialize(seed);
        traverseTree(firstStateIterator, results);
      }
    }

  private:
    /// Object pool for states
    StateArray m_states{};

    /// Subfindlet: hit selector
    AHitSelector m_hitSelector;

    /// Implementation of the traverseTree function
    void traverseTree(StateIterator currentState, std::vector<ResultPair>& resultsVector)
    {
      StateIterator nextState = std::next(currentState);

      if (nextState == m_states.end()) {
        resultsVector.emplace_back(currentState->finalize());
        return;
      }

      const auto& childStates = m_hitSelector.getChildStates(*currentState);

      if (childStates.empty()) {
        resultsVector.emplace_back(currentState->finalize());
        return;
      }

      for (AStateObject& childState : childStates) {
        std::swap(*nextState, childState);
        traverseTree(nextState, resultsVector);
      }
    }
  };
}