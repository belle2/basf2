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
#include <tracking/trackFindingCDC/collectors/base/ManyMatchCollector.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * A templated collector algorithm which only does the adding algorithm in cases where there is only one possibility
     * for a single collection item to be added to a collector item.
     */
    template <class AMatcherAlgorithm, class AAdderAlgorithm, class Compare = std::less<const typename AMatcherAlgorithm::CollectionItem*>>
    class SingleMatchCollector : public ManyMatchCollector<AMatcherAlgorithm, AAdderAlgorithm, Compare> {
    public:
      /// The super (parent) class.
      using Super = ManyMatchCollector<AMatcherAlgorithm, AAdderAlgorithm, Compare>;
      /// Copy the CollectorItem definition.
      using CollectorItem = typename Super::CollectorItem;
      /// Copy the CollectionItem definition.
      using CollectionItem = typename Super::CollectionItem;
      /// Copy the MatchedCollectionItem definition.
      using MatchedCollectionItem = typename Super::MatchedCollectionItem;

      /// Do the collection process.
      void collect(std::vector<CollectorItem>& collectorItems, const std::vector<CollectionItem>& collectionItems) override
      {
        const MapOfList<const CollectionItem*, WithWeight<CollectorItem*>, Compare>& matches = Super::constructMatchLookup(collectorItems,
            collectionItems);

        MapOfList<CollectorItem*, WithWeight<const CollectionItem*>> bestMatches;

        for (const auto& collectionItemWithMatchedCollectors : matches) {
          const CollectionItem* collectionItem = collectionItemWithMatchedCollectors.first;
          const std::vector<WithWeight<CollectorItem*>>& matchedCollectorItems = collectionItemWithMatchedCollectors.second;

          if (matchedCollectorItems.size() == 1) {
            const WithWeight<CollectorItem*>& singleElement = matchedCollectorItems[0];
            CollectorItem* bestCollectorItem = static_cast<CollectorItem*>(singleElement);
            const Weight bestWeight = singleElement.getWeight();

            WithWeight<const CollectionItem*> newElement(collectionItem, bestWeight);
            bestMatches.emplaceOrAppend(bestCollectorItem, newElement);
          }
        }

        Super::addMany(bestMatches);
      }
    };
  }
}
