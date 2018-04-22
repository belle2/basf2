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

#include <tracking/trackFindingCDC/numerics/WeightComperator.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <algorithm>
#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;
  namespace TrackFindingCDC {
    /**
     * Selector to remove all relations in the list, which share the same collection item - except the one which the
     * highest weight - and vice versa.
     *
     * This means the function will output a list of weighted relations, where the relations from collection to
     * collector items are bijective.
     *
     * Most likely, the full stack is used as follows:
     * * match two lists of elements (Collectors and Collections) resulting in a list of weighted relations among those
     * * select only some relations using a certain criteria (this is where the difference between Collector and Collections
     *   comes into place)
     * * add the found relations by absorbing the collection items into the collector items.
     *
     * Please note that the CollectionItems are therefore const whereas the CollectorItems are not. All the passed
     * WeightedRelations lists must be sorted.
     */
    template <class ACollectorItem, class ACollectionItem>
    class BestMatchSelector :
      public Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      using WeightedRelationItem = WeightedRelation<ACollectorItem, const ACollectionItem>;

      /// The parent class
      using Super = Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>>;

      /// Main function of this class doing the relation selection.
      void apply(std::vector<WeightedRelationItem>& weightedRelations) override
      {
        std::vector<WeightedRelationItem> selectedWeightedRelations;

        // Do until there are no more relations left:
        // (1) find the relation with the maximum weight
        // (2) store this and delete all relations, which share a "To" or a "From" item with this maximal relation
        // (3) repeat
        while (not weightedRelations.empty()) {
          // std:min_element is strange... this actually returns the element with the largest weight
          const auto maxElement = *(std::min_element(weightedRelations.begin(), weightedRelations.end(), GreaterWeight()));
          selectedWeightedRelations.push_back(maxElement);

          const auto itemSharesFromOrTo = [&maxElement](const WeightedRelationItem & item) {
            return item.getFrom() == maxElement.getFrom() or item.getTo() == maxElement.getTo();
          };

          erase_remove_if(weightedRelations, itemSharesFromOrTo);
        }

        std::sort(selectedWeightedRelations.begin(), selectedWeightedRelations.end());
        weightedRelations.swap(selectedWeightedRelations);
      }
    };
  }
}
