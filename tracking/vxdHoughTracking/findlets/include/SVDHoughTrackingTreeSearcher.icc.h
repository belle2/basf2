/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/findlets/SVDHoughTrackingTreeSearcher.dcl.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/Range.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    template <class AHit, class APathFilter, class AResult>
    SVDHoughTrackingTreeSearcher<AHit, APathFilter, AResult>::SVDHoughTrackingTreeSearcher() : Super()
    {
      Super::addProcessingSignalListener(&m_pathFilter);
    };

    template <class AHit, class APathFilter, class AResult>
    void SVDHoughTrackingTreeSearcher<AHit, APathFilter, AResult>::exposeParameters(ModuleParamList* moduleParamList,
        const std::string& prefix)
    {
      m_pathFilter.exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "twoHitFilterLimit"),
                                    m_applyTwoHitFilterIfMoreChildStates,
                                    "Use the TwoHitFilter (path length == 1) if there are more child states than this value.",
                                    m_applyTwoHitFilterIfMoreChildStates);
    }

    template <class AHit, class APathFilter, class AResult>
    void SVDHoughTrackingTreeSearcher<AHit, APathFilter, AResult>::apply(std::vector<AHit*>& hits,
        const std::vector<TrackFindingCDC::WeightedRelation<AHit>>& relations,
        std::vector<AResult>& results)
    {
      B2ASSERT("Expected relation to be sorted", std::is_sorted(relations.begin(), relations.end()));

      m_automaton.applyTo(hits, relations);

      std::vector<const AHit*> seedHits;
      for (const AHit* hit : hits) {
        if (hit->getDataCache().layer >= 5) {
          seedHits.emplace_back(hit);
        }
      }

      std::vector<TrackFindingCDC::WithWeight<const AHit*>> path;
      for (const AHit* seedHit : seedHits) {
        B2DEBUG(29, "Starting with new seed...");

        path.emplace_back(seedHit, 0);
        traverseTree(path, relations, results);
        path.pop_back();
        B2ASSERT("Something went wrong during the path traversal", path.empty());

        B2DEBUG(29, "... finished with seed");
      }
    }

    template <class AHit, class APathFilter, class AResult>
    void SVDHoughTrackingTreeSearcher<AHit, APathFilter, AResult>::traverseTree(std::vector<TrackFindingCDC::WithWeight<const AHit*>>&
        path,
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
        AHit* childHit = continuation.getTo();
        TrackFindingCDC::Weight weight = continuation.getWeight();
        // the state may still include information from an other round of processing, so lets set it back

        if (std::count(path.begin(), path.end(), childHit)) {
          // Cycle detected -- is this the best handling?
          // Other options: Raise an exception and bail out of this seed
          B2FATAL("Cycle detected!");
        }

        childHits.emplace_back(childHit, weight);
      }

      // Do everything with child states, linking, extrapolation, teaching, discarding, what have you.
      const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& constPath = path;
      if (path.size() > 1 or childHits.size() > m_applyTwoHitFilterIfMoreChildStates) {
        m_pathFilter.apply(constPath, childHits);
      }

      if (childHits.empty()) {
        B2DEBUG(29, "Terminating this route, as there are no possible child states.");
        if (path.size() >= 3) {
          results.emplace_back(path);
        }
        return;
      }

      // Traverse the tree from each new state on
      std::sort(childHits.begin(), childHits.end(), TrackFindingCDC::GreaterOf<TrackFindingCDC::GetWeight>());

      B2DEBUG(29, "Having found " << childHits.size() << " child states.");

      for (const TrackFindingCDC::WithWeight<AHit*>& childHit : childHits) {
        path.emplace_back(childHit, childHit.getWeight());
        traverseTree(path, relations, results);
        path.pop_back();
      }
    }
  }
}
