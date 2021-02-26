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

#include <tracking/datcon/optimizedDATCON/findlets/DATCONTreeSearcher.dcl.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/utilities/Range.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

namespace Belle2 {
//   template <class AState, class APathFilter, class AResult>
//   DATCONTreeSearcher<AState, APathFilter, AResult>::DATCONTreeSearcher() : Super()
  template <class AState, class APathFilter>
  DATCONTreeSearcher<AState, APathFilter>::DATCONTreeSearcher() : Super()
  {
    Super::addProcessingSignalListener(&m_pathFilter);
  };

//   template <class AState, class APathFilter, class AResult>
//   void DATCONTreeSearcher<AState, APathFilter, AResult>::exposeParameters(ModuleParamList* moduleParamList,
//       const std::string& prefix)
  template <class AState, class APathFilter>
  void DATCONTreeSearcher<AState, APathFilter>::exposeParameters(ModuleParamList* moduleParamList,
      const std::string& prefix)
  {
    m_pathFilter.exposeParameters(moduleParamList, prefix);
  }

//   template <class AState, class APathFilter, class AResult>
//   void DATCONTreeSearcher<AState, APathFilter, AResult>::apply(const std::vector<AState>& seededStates,
//       std::vector<AState>& hitStates,
//       const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations,
//       std::vector<AResult>& results)
  template <class AState, class APathFilter>
  void DATCONTreeSearcher<AState, APathFilter>::apply(const std::vector<AState>& seededStates,
                                                      std::vector<AState>& hitStates,
                                                      const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations)
  {
    B2ASSERT("Expected relation to be sorted",
             std::is_sorted(relations.begin(), relations.end()));

    // TODO: May be better to just do this for each seed separately
    const std::vector<AState*>& statePointers = TrackFindingCDC::as_pointers<AState>(hitStates);
    m_automaton.applyTo(statePointers, relations);

    std::vector<TrackFindingCDC::WithWeight<const AState*>> path;
    for (const AState& state : seededStates) {
      B2DEBUG(29, "Starting with new seed...");

      path.emplace_back(&state, 0);
//       traverseTree(path, relations, results);
      traverseTree(path, relations);
      path.pop_back();
      B2ASSERT("Something went wrong during the path traversal", path.empty());

      B2DEBUG(29, "... finished with seed");
    }
  }

//   template <class AState, class APathFilter, class AResult>
//   void DATCONTreeSearcher<AState, APathFilter, AResult>::traverseTree(std::vector<TrackFindingCDC::WithWeight<const AState*>>&
//       path,
//       const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations,
//       std::vector<AResult>& results)
  template <class AState, class APathFilter>
  void DATCONTreeSearcher<AState, APathFilter>::traverseTree(std::vector<TrackFindingCDC::WithWeight<const AState*>>&
                                                             path,
                                                             const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations)
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

      if (std::count(path.begin(), path.end(), childState)) {
        // Cycle detected -- is this the best handling?
        // Other options: Raise an exception and bail out of this seed
        B2FATAL("Cycle detected!");
      }

      childStates.emplace_back(childState, weight);
    }

    // Apply three-hit-filters, so the path has to contain at least to hits to form >= triplet with the child states
    if (path.size() > 2) {
      // Do everything with child states, linking, extrapolation, teaching, discarding, what have you.
      const std::vector<TrackFindingCDC::WithWeight<const AState*>>& constPath = path;
//       m_pathFilter.apply(constPath, childStates);

      if (childStates.empty()) {
        B2DEBUG(29, "Terminating this route, as there are no possible child states.");
        return;
      }
    }

    // Traverse the tree from each new state on
    const auto stateLess = [](const auto & lhs, const auto & rhs) {
      return lhs->getAutomatonCell().getCellState() < rhs->getAutomatonCell().getCellState();
    };
    std::sort(childStates.begin(), childStates.end(), stateLess);

    B2DEBUG(29, "Having found " << childStates.size() << " child states.");
    for (const TrackFindingCDC::WithWeight<AState*>& childState : childStates) {
      path.emplace_back(childState, childState.getWeight());
//       traverseTree(path, relations, results);
      traverseTree(path, relations);
      path.pop_back();
    }
  }
}
