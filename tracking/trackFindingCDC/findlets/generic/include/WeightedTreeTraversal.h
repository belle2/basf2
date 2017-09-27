/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/Range.h>

#include <framework/logging/Logger.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;
  namespace TrackFindingCDC {
    /**
     * General implementation of a tree search algorithm using a given classes as state and results
     * and one strategy class to decide, which child states should be traversed next.
     *
     * Requirements for the template parameters:
     * - AState:
     *     No requirements
     * - AResult:
     *     Must be constructable from std::vector<const AState*>
     * - AStateRejecter:
     *     Should be roughly of type Findlet<const AState* const, WithWeight<AState*> >
     *     with an apply methode like:
     *
     *     apply(const std::vector<const AState*>& currentPath,
     *           std::vector<WithWeight<AState*> >& nextStates);
     *
     *     Therefore it receives the current path and a list of potential next states with an
     *     apriori Weight, that it should sort out for viable continuations to traverse next.
     *
     *     The StateRejecter is allowed to adjust the nextStates for book keeping purposes, but
     *     should generally avoid to adjust the states in the currentPath to not interfere with the
     *     traversal of earlier siblings.
     */
    template <class AStateRejecter, class AState, class AResult = std::vector<const AState*>>
    class WeightedTreeTraversal
      : public Findlet<const AState* const, const WeightedRelation<AState>, AResult> {
    private:
      /// Parent class
      using Super = Findlet<const AState* const, const WeightedRelation<AState>, AResult>;

    public:
      /// Construct this findlet and add the subfindlet as listener
      WeightedTreeTraversal()
        : Super()
      {
        Super::addProcessingSignalListener(&m_stateRejecter);
      }

      /// Expose the parameters of the subfindlet
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_stateRejecter.exposeParameters(moduleParamList, prefix);
      }

      /**
       * Main function of this findlet: traverse a tree starting from a given seed states.
       *
       * ATTENTION: As described above, the states themselves can be altered during the tree
       * traversal.
       */
      void apply(const std::vector<const AState*>& seededStates,
                 const std::vector<WeightedRelation<AState>>& stateRelations,
                 std::vector<AResult>& results) override;

    private:
      /// Implementation of the traverseTree function
      void traverseTree(std::vector<const AState*>& path,
                        const std::vector<WeightedRelation<AState>>& stateRelations,
                        std::vector<AResult>& results);

    private:
      /// State rejecter to decide which available continuations should be traversed next.
      AStateRejecter m_stateRejecter;
    };

    template <class AStateRejecter, class AState, class AResult>
    void WeightedTreeTraversal<AStateRejecter, AState, AResult>::apply(
      const std::vector<const AState*>& seededStates,
      const std::vector<WeightedRelation<AState>>& stateRelations,
      std::vector<AResult>& results)
    {
      std::vector<const AState*> path;
      for (const AState* state : seededStates) {
        B2DEBUG(50, "Starting with new seed...");
        path.push_back(state);
        traverseTree(path, stateRelations, results);
        path.pop_back();
        B2DEBUG(50, "... finished with seed");
      }
      assert(path.empty());
    }

    template <class AStateRejecter, class AState, class AResult>
    void WeightedTreeTraversal<AStateRejecter, AState, AResult>::traverseTree(
      std::vector<const AState*>& path,
      const std::vector<WeightedRelation<AState>>& stateRelations,
      std::vector<AResult>& results)
    {
      // Implement only graph traversal logic and leave the extrapolation and selection to the
      // rejecter.
      const AState* currentState = path.back();
      auto continuations =
        std::equal_range(stateRelations.begin(), stateRelations.end(), currentState);

      std::vector<WithWeight<AState*>> childStates;
      for (const WeightedRelation<AState>& continuation : asRange(continuations)) {
        AState* childState = continuation.getTo();
        Weight weight = continuation.getWeight();
        childStates.push_back({childState, weight});
      }

      // Do everything with child states, linking, extrapolation, teaching, discarding, what have
      // you.
      const std::vector<const AState*>& constPath = path;
      m_stateRejecter.apply(constPath, childStates);

      if (childStates.empty()) {
        B2DEBUG(50, "Terminating this route, as there are no possible child states.");
        results.emplace_back(path);
        return;
      }

      // Traverse the tree from each new state on
      B2DEBUG(50, "Having found " << childStates.size() << " child states.");
      for (WithWeight<AState*> childState : childStates) {
        if (std::count(path.begin(), path.end(), childState)) {
          // Cycle detected -- is this the best handling?
          // Other options: Raise and exception and bail out of this seed
          continue;
        }
        path.push_back(childState);
        traverseTree(path, stateRelations, results);
        path.pop_back();
      }
    }
  }
}
