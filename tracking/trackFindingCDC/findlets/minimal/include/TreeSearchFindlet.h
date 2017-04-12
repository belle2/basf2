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

#include <tracking/trackFindingCDC/utilities/SortedVectorRange.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class SeedObject, class HitObject, class StateObject>
    class TreeSearchFindlet : public Findlet<SeedObject*, const HitObject*> {
    public:
      using SeedPtr = SeedObject*;
      using HitPtr = const HitObject*;
      using StateArray = typename std::array<StateObject, StateObject::N>;
      using StateIterator = typename StateArray::iterator;

      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        Findlet<SeedObject*, const HitObject*>::exposeParameters(moduleParamList, prefix);

        moduleParamList->addParameter(prefixed(prefix, "makeHitJumpingPossible"), m_param_makeHitJumpingPossible,
                                      "", m_param_makeHitJumpingPossible);
      }

      void traverseTree(SeedPtr seed, std::vector<std::pair<SeedPtr, std::vector<HitPtr>>>& resultsVector)
      {
        m_states.front().initialize(seed);
        traverseTree(m_states.begin(), resultsVector);
      }


    protected:
      virtual TrackFindingCDC::SortedVectorRange<HitPtr> getMatchingHits(StateIterator currentState) = 0;
      virtual bool useResult(StateIterator currentState) = 0;

    private:
      StateArray m_states{};
      bool m_param_makeHitJumpingPossible = true;

      void traverseTree(StateIterator currentState,
                        std::vector<std::pair<SeedPtr, std::vector<HitPtr>>>& resultsVector)
      {
        const auto& matchingHits = getMatchingHits(currentState);
        StateIterator nextState = std::next(currentState);

        if (nextState == m_states.end() or (not m_param_makeHitJumpingPossible and matchingHits.empty())) {
          resultsVector.emplace_back(currentState->finalize());
          return;
        }

        for (const auto& hit : matchingHits) {
          nextState->buildFrom(currentState, hit);

          if (not useResult(nextState)) {
            continue;
          }

          traverseTree(nextState, resultsVector);
        }

        if (m_param_makeHitJumpingPossible) {
          nextState->buildFrom(currentState, nullptr);

          if (useResult(nextState)) {
            traverseTree(nextState, resultsVector);
          }

        }
      }
    };
  }
}