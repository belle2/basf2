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
     * A templated collector algorithm which uses the weight information attached to the return-list from the matcher, to search for the
     * single best CollectorItem for every CollectionItem.
     */
    template <class AMatherAlgorithm, class AAdderAlgorithm>
    class BestMatchCollector : public ManyMatchCollector<AMatherAlgorithm, AAdderAlgorithm> {
    public:
      /// The super (parent) class.
      using Super = ManyMatchCollector<AMatherAlgorithm, AAdderAlgorithm>;
      /// Copy the CollectorItem definition.
      using CollectorItem = typename Super::CollectorItem;
      /// Copy the CollectionItem definition.
      using CollectionItem = typename Super::CollectionItem;
      /// Copy the MatchedCollectionItem definition.
      using MatchedCollectionItem = typename Super::MatchedCollectionItem;

      /// Do the collection process.
      virtual void collect(std::vector<CollectorItem>& collectorItems, const std::vector<CollectionItem>& collectionItems) override
      {
        const MapOfList<const CollectionItem*, WithWeight<CollectorItem*>>& matches = Super::constructMatchLookup(collectorItems,
            collectionItems);
        const MapOfList<CollectorItem*, WithWeight<const CollectionItem*>>& bestMatches = findBestMatches(matches);

        Super::addMany(bestMatches);
      }

    private:
      /// Find the best match for each collection item.
      const MapOfList<CollectorItem*, WithWeight<const CollectionItem*>> findBestMatches(const
          MapOfList<const CollectionItem*, WithWeight<CollectorItem*>>& matches)
      {
        // For each collection item, find the best collector and transpose the relation again (but only keeping these best candidates).
        MapOfList<CollectorItem*, WithWeight<const CollectionItem*>> bestMatches;
        for (auto& matchedCollectorItemsWithWeights : matches) {
          const CollectionItem* collectionItem = matchedCollectorItemsWithWeights.first;
          const std::vector<WithWeight<CollectorItem*>>& collectorItemsWithWeights = matchedCollectorItemsWithWeights.second;

          auto maximalElement = std::max_element(collectorItemsWithWeights.begin(),
          collectorItemsWithWeights.end(), [](const WithWeight<CollectorItem*>& lhs, const WithWeight<CollectorItem*>& rhs) {
            return lhs.getWeight() < rhs.getWeight();
          });

          CollectorItem* bestCollectorItem = static_cast<CollectorItem*>(*maximalElement);
          const Weight bestWeight = maximalElement->getWeight();

          WithWeight<const CollectionItem*> newElement(collectionItem, bestWeight);
          bestMatches.emplaceOrAppend(bestCollectorItem, newElement);
        }

        return bestMatches;
      }
    };
  }
}
