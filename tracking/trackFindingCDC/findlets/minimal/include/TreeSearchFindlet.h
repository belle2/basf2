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

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/trackFindingCDC/utilities/SortedVectorRange.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    template<class AStateObject, class AFilterFactory>
    class TreeSearchFindlet : public Findlet<typename AStateObject::SeedObject*, const typename AStateObject::HitObject*> {
    public:
      using Super = Findlet<typename AStateObject::SeedObject*, const typename AStateObject::HitObject*>;
      using SeedPtr = typename AStateObject::SeedObject*;
      using HitPtr = const typename AStateObject::HitObject*;
      using StateArray = typename std::array<AStateObject, AStateObject::N>;
      using StateIterator = typename StateArray::iterator;

      /// Construct this findlet and add the two filters as listeners
      TreeSearchFindlet() : Super()
      {
        Super::addProcessingSignalListener(&m_firstFilter);
        Super::addProcessingSignalListener(&m_secondFilter);
      }

      /// Expose the parameters of the two filters and the makeHitJumpsPossible parameter
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        Super::exposeParameters(moduleParamList, prefix);

        m_firstFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "first"));
        m_secondFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "second"));

        moduleParamList->addParameter(prefixed(prefix, "makeHitJumpingPossible"), m_param_makeHitJumpingPossible,
                                      "", m_param_makeHitJumpingPossible);
      }

      /// Main function of this findlet: traverse a tree starting from a given seed object.
      void traverseTree(SeedPtr seed, std::vector<std::pair<SeedPtr, std::vector<HitPtr>>>& resultsVector)
      {
        // TODO: Pull this into the apply function!
        m_states.front().initialize(seed);
        traverseTree(m_states.begin(), resultsVector);
      }

    protected:
      /// Overloadable function to return the possible range of next hit objects for a given state
      virtual TrackFindingCDC::SortedVectorRange<HitPtr> getMatchingHits(StateIterator currentState) = 0;

    private:
      /// Object cache for states
      StateArray m_states{};

      /// Parameter: make hit jumps possible
      bool m_param_makeHitJumpingPossible = true;

      /// Subfindlet: Filter 1
      ChooseableFilter<AFilterFactory> m_firstFilter;
      /// Subfindlet: Filter 2
      ChooseableFilter<AFilterFactory> m_secondFilter;

      /// Implementation of the traverseTree function
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

      /// Test function whether a given state should be used or not
      bool useResult(StateIterator currentState)
      {
        // Check the first filter before extrapolation
        TrackFindingCDC::Weight weight = m_firstFilter(*currentState);
        if (std::isnan(weight)) {
          return false;
        }

        // Extrapolate
        currentState->advance();

        // Check the second filter after extrapolation
        weight = m_secondFilter(*currentState);
        return not std::isnan(weight);
      }
    };
  }
}