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
     * Selector to remove all weighted relations, where a definable Filter
     * gives NaN as a result. Will also update all stored weights with the result of the filter.
     * Please note that all other stored weights will be overriden!
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
    template <class ACollectorItem, class ACollectionItem, class AFilter>
    class FilterSelector :
      public Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      using WeightedRelationItem = WeightedRelation<ACollectorItem, const ACollectionItem>;

      /// The parent class
      using Super = Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>>;

      /// Add the chosen filter as a process signal listener.
      FilterSelector() : Super()
      {
        Super::addProcessingSignalListener(&m_filter);
      }

      /// Expose the parameters of the filter.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        Super::exposeParameters(moduleParamList, prefix);
        m_filter.exposeParameters(moduleParamList, prefix);
      }

      /// Main function of the class: calculate the filter result and remove all relations, where the filter returns NaN
      void apply(std::vector<WeightedRelationItem>& weightedRelations) override
      {
        for (WeightedRelationItem& weightedRelation : weightedRelations) {
          const Weight weight = m_filter(weightedRelation);
          weightedRelation.setWeight(weight);
        }

        const auto& weightIsNan = [](const WeightedRelationItem & item) {
          return std::isnan(item.getWeight());
        };

        // Erase all items with a weight of NAN
        erase_remove_if(weightedRelations, weightIsNan);

        std::sort(weightedRelations.begin(), weightedRelations.end());
      }

    private:
      /// The filter to use.
      AFilter m_filter;
    };
  }
}
