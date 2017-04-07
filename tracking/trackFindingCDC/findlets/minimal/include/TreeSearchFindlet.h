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

      virtual void apply(std::vector<SeedPtr>& seedsVector,
                         std::vector<HitPtr>& filteredHitVector) override
      {
        // Maybe implement some caching here.
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

      void traverseTree(StateIterator currentState,
                        std::vector<std::pair<SeedPtr, std::vector<HitPtr>>>& resultsVector)
      {
        const auto& matchingHits = getMatchingHits(currentState);
        StateIterator nextState = std::next(currentState);

        if (nextState == m_states.end() or matchingHits.empty()) {
          resultsVector.emplace_back(currentState->finalize());
          return;
        }

        for (const auto& hit : matchingHits) {
          nextState->buildFrom(currentState, hit);

          if (not useResult(nextState)) {
            continue;
          }

          traverseTree(nextState, resultsVector);
          //return;
        };
      }
    };
  }
}