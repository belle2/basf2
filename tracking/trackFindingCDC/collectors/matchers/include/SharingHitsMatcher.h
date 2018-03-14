/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/collectors/matchers/MatcherInterface.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <map>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /**
     * A generic matcher algorithm which outputs all combinations of elements
     * with the number of shared hits as a weight.
     *
     * It does only use those collection items, which do not have a taken flag.
     */
    template <class ACollectorItem, class ACollectionItem>
    class SharingHitsMatcher : public MatcherInterface<ACollectorItem, ACollectionItem> {

      /// The parent class.
      using Super = MatcherInterface<ACollectorItem, ACollectionItem>;

    public:
      /// Clear the hit lookup in every event
      void beginEvent() override
      {
        m_hitLookup.clear();
      }

      /// Call the apply function of the super class - but before fill the hit lookup.
      void apply(std::vector<ACollectorItem>& collectorItems, const std::vector<ACollectionItem>& collectionItems,
                 std::vector<typename Super::WeightedRelationItem>& weightedRelations) override
      {

        for (const ACollectionItem& collectionItem : collectionItems) {
          const AutomatonCell& automatonCell = collectionItem.getAutomatonCell();

          if (automatonCell.hasTakenFlag() or collectionItem.empty()) {
            continue;
          }

          for (const auto& hit : collectionItem) {
            const CDCWireHit& wireHit = hit.getWireHit();

            m_hitLookup.insert(std::make_pair(&wireHit, &collectionItem));
          }
        }

        Super::apply(collectorItems, collectionItems, weightedRelations);
      }

    private:
      /**
       * Output the number of shared hits - the rest of the logic (relation creation etc.)
       * is handled by the parent class.
       */
      void match(ACollectorItem& collectorItem, const std::vector<ACollectionItem>& collectionItems,
                 std::vector<typename Super::WeightedRelationItem>& relationsForCollector) override
      {
        std::map<const ACollectionItem*, unsigned int> numberOfIntersectionsMap;

        // Set the number of intersections to 0
        for (const ACollectionItem& collectionItem : collectionItems) {
          numberOfIntersectionsMap[&collectionItem] = 0;
        }

        // For each hit in the collector item: Get all collection items for this hit and increase the number of intersections
        for (const auto& hit : collectorItem) {
          const CDCWireHit& wireHit = hit.getWireHit();

          // Get all matched collection items
          const auto& relatedCollectionItems = m_hitLookup.equal_range(&wireHit);

          for (auto& relatedCollectionItemIterator : asRange(relatedCollectionItems)) {
            const ACollectionItem* collectionItem = relatedCollectionItemIterator.second;
            numberOfIntersectionsMap[collectionItem] += 1;
          }
        }

        // Add new relations to the list
        for (const auto& collectionWithIntersections : numberOfIntersectionsMap) {
          const ACollectionItem* collectionItem = collectionWithIntersections.first;
          unsigned int numberOfIntersections = collectionWithIntersections.second;

          relationsForCollector.emplace_back(&collectorItem, numberOfIntersections, collectionItem);
        }
      }

      std::multimap<const CDCWireHit*, const ACollectionItem*> m_hitLookup;
    };
  }
}
