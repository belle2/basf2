/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/TreeSearcher.dcl.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingCDC/utilities/Range.h>

namespace Belle2 {
  template <class AState, class AStateRejecter, class AResult>
  TreeSearcher<AState, AStateRejecter, AResult>::TreeSearcher() : Super()
  {
    Super::addProcessingSignalListener(&m_stateRejecter);
  };

  template <class AState, class AStateRejecter, class AResult>
  void TreeSearcher<AState, AStateRejecter, AResult>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_stateRejecter.exposeParameters(moduleParamList, prefix);
  }

  template <class AState, class AStateRejecter, class AResult>
  void TreeSearcher<AState, AStateRejecter, AResult>::apply(const std::vector<AState>& seededStates,
                                                            const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations,
                                                            std::vector<AResult>& results)
  {
    B2ASSERT("Expected relation to be sorted",
             std::is_sorted(relations.begin(), relations.end()));

    std::vector<const AState*> path;
    for (const AState& state : seededStates) {
      B2DEBUG(50, "Starting with new seed...");

      path.push_back(&state);
      traverseTree(path, relations, results);
      path.pop_back();
      B2ASSERT("Something went wrong during the path traversal", path.empty());

      B2DEBUG(50, "... finished with seed");
    }
  }

  template <class AState, class AStateRejecter, class AResult>
  void TreeSearcher<AState, AStateRejecter, AResult>::traverseTree(std::vector<const AState*>& path,
      const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations,
      std::vector<AResult>& results)
  {
    // Implement only graph traversal logic and leave the extrapolation and selection to the
    // rejecter.
    const AState* currentState = path.back();
    auto continuations =
      TrackFindingCDC::asRange(std::equal_range(relations.begin(), relations.end(), currentState));

    std::vector<TrackFindingCDC::WithWeight<AState*>> childStates;
    for (const TrackFindingCDC::WeightedRelation<AState>& continuation : continuations) {
      AState* childState = continuation.getTo();
      TrackFindingCDC::Weight weight = continuation.getWeight();
      // the state may still include information from an other round of processing, so lets set it back
      childState->reset();
      childStates.emplace_back(childState, weight);
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
    for (const AState* childState : childStates) {
      if (std::count(path.begin(), path.end(), childState)) {
        // Cycle detected -- is this the best handling?
        // Other options: Raise an exception and bail out of this seed
        B2WARNING("Cycle detected!");
        continue;
      }
      path.push_back(childState);
      traverseTree(path, relations, results);
      path.pop_back();
    }
  }
}
