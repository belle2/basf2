/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Base class for a findlet, which uses a reduced/thinned list of weighted relations between
     * collector and collection items to absorb the matched collection items into a collector item.
     *
     * Please do not implement any logic which collection items should be added to which collector item,
     * but handle all this in a selector (see below).
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
    class AdderInterface :
      public Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>&> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      using WeightedRelationItem = WeightedRelation<ACollectorItem, const ACollectionItem>;

      /// Main function to do the adding. Override the add function below in your derived class.
      void apply(std::vector<WeightedRelationItem>& weightedRelations) override
      {
        for (const WeightedRelationItem& relationItem : weightedRelations) {
          ACollectorItem& collectorItem = *relationItem.getFrom();
          const ACollectionItem& collectionItem = *relationItem.getTo();
          Weight weight = relationItem.getWeight();
          add(collectorItem, collectionItem, weight);
        }
      }

    protected:
      /// Override this function to implement how a collection item should be matched to a collector item.
      virtual void add(ACollectorItem& collectorItem __attribute__((unused)),
                       const ACollectionItem& collectionItem __attribute__((unused)),
                       Weight weight __attribute__((unused)))
      {
      }
    };
  }
}
