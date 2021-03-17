/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
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
  template <class AHit, class APathFilter, class AResult>
  DATCONTreeSearcher<AHit, APathFilter, AResult>::DATCONTreeSearcher() : Super()
  {
    Super::addProcessingSignalListener(&m_pathFilter);
  };

  template <class AHit, class APathFilter, class AResult>
  void DATCONTreeSearcher<AHit, APathFilter, AResult>::exposeParameters(ModuleParamList* moduleParamList,
      const std::string& prefix)
  {
    m_pathFilter.exposeParameters(moduleParamList, prefix);
  }

  template <class AHit, class APathFilter, class AResult>
  void DATCONTreeSearcher<AHit, APathFilter, AResult>::apply(std::vector<AHit*>& hits,
                                                             const std::vector<TrackFindingCDC::WeightedRelation<AHit>>& relations,
                                                             std::vector<AResult>& results)
  {
    B2ASSERT("Expected relation to be sorted",
             std::is_sorted(relations.begin(), relations.end()));

//     // TODO: May be better to just do this for each seed separately
//     const std::vector<AHit*>& hitPointers = TrackFindingCDC::as_pointers<AHit>(hits);
    // TODO: Maybe const-cast as above, which was done by Nils for the CKF, where he
    //       starts with a std::vector<AHit> instead of std::vector<AHit*> though...
//     const std::vector<AHit*>& hitPointers = hits;
//     m_automaton.applyTo(hitPointers, relations);
    m_automaton.applyTo(hits, relations);

    std::vector<TrackFindingCDC::WithWeight<const AHit*>> path;
    for (const AHit* hit : hits) {
      B2DEBUG(29, "Starting with new seed...");

      path.emplace_back(hit, 0);
      traverseTree(path, relations, results);
      path.pop_back();
      B2ASSERT("Something went wrong during the path traversal", path.empty());

      B2DEBUG(29, "... finished with seed");
    }
  }

  template <class AHit, class APathFilter, class AResult>
  void DATCONTreeSearcher<AHit, APathFilter, AResult>::traverseTree(std::vector<TrackFindingCDC::WithWeight<const AHit*>>& path,
      const std::vector<TrackFindingCDC::WeightedRelation<AHit>>& relations,
      std::vector<AResult>& results)
  {
    // Implement only graph traversal logic and leave the extrapolation and selection to the
    // rejecter.
    const AHit* currentHit = path.back();
    auto continuations =
      TrackFindingCDC::asRange(std::equal_range(relations.begin(), relations.end(), currentHit));

    std::vector<TrackFindingCDC::WithWeight<AHit*>> childHits;
    for (const TrackFindingCDC::WeightedRelation<AHit>& continuation : continuations) {
      AHit* childState = continuation.getTo();
      TrackFindingCDC::Weight weight = continuation.getWeight();
      // the state may still include information from an other round of processing, so lets set it back

      if (std::count(path.begin(), path.end(), childState)) {
        // Cycle detected -- is this the best handling?
        // Other options: Raise an exception and bail out of this seed
        B2FATAL("Cycle detected!");
      }

      childHits.emplace_back(childState, weight);
    }

    // Apply three-hit-filters, so the path has to contain at least to hits to form >= triplet with the child states
    if (path.size() >= 2) {
      // Do everything with child states, linking, extrapolation, teaching, discarding, what have you.
      const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& constPath = path;
      m_pathFilter.apply(constPath, childHits);
    }

    if (childHits.empty() and path.size() >= 3) {
      B2DEBUG(29, "Terminating this route, as there are no possible child states.");
      results.emplace_back(path);
      return;
    }

    // Traverse the tree from each new state on
    const auto stateLess = [](const auto & lhs, const auto & rhs) {
      return lhs->getAutomatonCell().getCellState() < rhs->getAutomatonCell().getCellState();
    };
    std::sort(childHits.begin(), childHits.end(), stateLess);

    B2DEBUG(29, "Having found " << childHits.size() << " child states.");
    for (const TrackFindingCDC::WithWeight<AHit*>& childState : childHits) {
      path.emplace_back(childState, childState.getWeight());
      traverseTree(path, relations, results);
      path.pop_back();
    }
  }
}
