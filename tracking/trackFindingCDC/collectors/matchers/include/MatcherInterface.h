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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Base class for a findlet, which outputs a list of weighted relations between
     * elements in a list of CollectorItems and a list of CollectionItems.
     *
     * Which item pairs are related and with which weight (if at all), can be defined by
     * overriding one of the match()-functions in a derived class.
     *
     * Most likely, the full stack is used as follows:
     * * match two lists of elements (Collectors and Collections) resulting in a list of weighted relations among those
     * * select only some relations using a certain criteria (this is where the difference between Collector and Collections
     *   comes into place)
     * * add the found relations by absorbing the collection items into the collector items.
     *
     * Please note that the CollectionItems are therefore const whereas the CollectorItems are not. All the passed
     * WeightedRelations lists must be sorted.
     *
     * Most of the provided selectors are built to match many collection items to one collector item.
     */
    template <class ACollectorItem, class ACollectionItem>
    class MatcherInterface :
      public Findlet<ACollectorItem,
      const ACollectionItem,
      WeightedRelation<ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      using WeightedRelationItem = WeightedRelation<ACollectorItem, const ACollectionItem>;

      /**
       * Main function: create weighted relations between collectors and collection items using
       * the implemented functionality overriden in a derived class.
       */
      void apply(std::vector<ACollectorItem>& collectorItems, const std::vector<ACollectionItem>& collectionItems,
                 std::vector<WeightedRelationItem>& weightedRelations) override
      {
        for (ACollectorItem& collectorItem : collectorItems) {
          match(collectorItem, collectionItems, weightedRelations);
        }

        std::sort(weightedRelations.begin(), weightedRelations.end());
      }

    protected:
      /**
       * Override this function to implement your own matching algorithm between one collector and
       * many collection items. A reason to override this instead of the other match function could be, if you want
       * to apply some sort of caching for each collector item.
       */
      virtual void match(ACollectorItem& collectorItem, const std::vector<ACollectionItem>& collectionItems,
                         std::vector<WeightedRelationItem>& relationsForCollector)
      {
        for (const ACollectionItem& collectionItem : collectionItems) {
          Weight weight = match(collectorItem, collectionItem);
          relationsForCollector.emplace_back(&collectorItem, weight, &collectionItem);
        }
      };

      /**
       * Override this function to implement your own matching algorithm between one collector and
       * one collection items. If you override this function, you just have to return a weight - the rest of the
       * logic is handled by the MatcherInterface class.
       */
      virtual Weight match(ACollectorItem& collectorItem __attribute__((unused)),
                           const ACollectionItem& collectionItem __attribute__((unused)))
      {
        return NAN;
      }
    };
  }
}
