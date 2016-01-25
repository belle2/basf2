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
#include <tracking/trackFindingCDC/collectors/base/MapOfList.h>
#include <tracking/trackFindingCDC/collectors/base/Collector.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * A templated collector algorithm which uses the weight information attached to the return-list from the matcher, to search for the
     * single best CollectorItem for every CollectionItem.
     */
    template <class AMatherAlgorithm, class AAdderAlgorithm>
    class BestMatchCollector : public Collector<AMatherAlgorithm, AAdderAlgorithm> {
    public:
      /// The super (parent) class.
      typedef Collector<AMatherAlgorithm, AAdderAlgorithm> Super;
      /// Copy the CollectorItem definition.
      typedef typename Super::CollectorItem CollectorItem;
      /// Copy the CollectionItem definition.
      typedef typename Super::CollectionItem CollectionItem;
      /// Copy the MatchedCollectionItem definition.
      typedef typename Super::MatchedCollectionItem MatchedCollectionItem;

      /// Do the collection process.
      virtual void collect(std::vector<CollectorItem>& collectorItems, const std::vector<CollectionItem>& collectionItems) override
      {
        MapOfList<const CollectionItem*, WithWeight<CollectorItem*>> matches;

        for (CollectorItem& collectorItem : collectorItems) {
          const std::vector<MatchedCollectionItem>& matchesForCollector = this->m_matcherInstance.match(collectorItem, collectionItems);

          // "Transpose" the relation collector -> weighted collections to collection -> weighted collectors
          for (const auto& matchedCollectionItemWithWeight : matchesForCollector) {
            const CollectionItem* collectionItem = matchedCollectionItemWithWeight;
            const Weight weight = matchedCollectionItemWithWeight.getWeight();

            WithWeight<CollectorItem*> newElement(&collectorItem, weight);
            matches.emplaceOrAppend(collectionItem, newElement);
          }
        }

        // For each collection item, find the best collector and transpose the relation again (but only keeping these best candidates).
        MapOfList<CollectorItem*, WithWeight<const CollectionItem*>> filteredMatches;
        for (auto& matchedCollectorItemsWithWeights : matches) {
          const CollectionItem* collectionItem = matchedCollectorItemsWithWeights.first;
          std::vector<WithWeight<CollectorItem*>>& collectorItemsWithWeights = matchedCollectorItemsWithWeights.second;

          auto maximalElement = std::max_element(collectorItemsWithWeights.begin(),
          collectorItemsWithWeights.end(), [](const WithWeight<CollectorItem*>& lhs, const WithWeight<CollectorItem*>& rhs) {
            return lhs.getWeight() < rhs.getWeight();
          });

          CollectorItem* bestCollectorItem = *maximalElement;
          const Weight bestWeight = maximalElement->getWeight();

          WithWeight<const CollectionItem*> newElement(collectionItem, bestWeight);
          filteredMatches.emplaceOrAppend(bestCollectorItem, newElement);
        }

        // In the end, we can add them perfectly easy :-)
        for (auto& collectorItemWithMatches : filteredMatches) {
          CollectorItem* collectorItem = collectorItemWithMatches.first;
          const std::vector<WithWeight<const CollectionItem*>>& matchedCollectionItems = collectorItemWithMatches.second;

          AAdderAlgorithm::add(*collectorItem, matchedCollectionItems);
        }
      }
    };
  }
}
