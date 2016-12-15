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

namespace Belle2 {
  namespace TrackFindingCDC {
    template <class ACollectorItem, class ACollectionItem>
    class AdderInterface :
      public Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>&> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      using WeightedRelationItem = WeightedRelation<ACollectorItem, const ACollectionItem>;

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
      virtual void add(ACollectorItem& collectorItem, const ACollectionItem& collectionItem, Weight weight)
      {
      }
    };
  }
}
