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
     * A templated collector algorithm which does not use the first match but runs the matching algorithm with all collection/collector
     * item pairs to search for the best etc. afterwards.
     */
    template <class AMatherAlgorithm, class AAdderAlgorithm>
    class ManyMatchCollector : public Collector<AMatherAlgorithm, AAdderAlgorithm> {
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
      virtual void collect(std::vector<CollectorItem>& collectorItems, const std::vector<CollectionItem>& collectionItems) override = 0;

    protected:
      /**
       * Helper function for testing each pair of collector-collection-items with the match function.
       * It returns a map collectionItem->list of possible collector item matches with weight.
      */
      const MapOfList<const CollectionItem*, WithWeight<CollectorItem*>> constructMatchLookup(std::vector<CollectorItem>& collectorItems,
          const std::vector<CollectionItem>& collectionItems)
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

        return matches;
      }

      /// Helper function for adding all the found and matched collection items of a collector to the collector item.
      void addMany(const MapOfList<CollectorItem*, WithWeight<const CollectionItem*>>& matches) const
      {
        // In the end, we can add them perfectly easy :-)
        for (auto& collectorItemWithMatches : matches) {
          CollectorItem* collectorItem = collectorItemWithMatches.first;
          const std::vector<WithWeight<const CollectionItem*>>& matchedCollectionItems = collectorItemWithMatches.second;

          AAdderAlgorithm::add(*collectorItem, matchedCollectionItems);
        }
      }
    };
  }
}
