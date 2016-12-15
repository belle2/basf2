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

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    template <class ACollectorItem, class ACollectionItem, class AFilterFactory>
    class FilterSelector :
      public Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      using WeightedRelationItem = WeightedRelation<ACollectorItem, const ACollectionItem>;

      /// The parent class
      using Super = Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>>;

      FilterSelector() : Super()
      {
        Super::addProcessingSignalListener(&m_filter);
      }

      void exposeParameters(ModuleParamList* moduleParamList,
                            const std::string& prefix) override
      {
        Super::exposeParameters(moduleParamList, prefix);
        m_filter.exposeParameters(moduleParamList, prefix);
      }

      void apply(std::vector<WeightedRelationItem>& weightedRelations) override
      {
        for (auto& weightedRelation : weightedRelations) {
          Weight filterResult = m_filter({ weightedRelation.getFrom(), weightedRelation.getTo() });
          weightedRelation.setWeight(filterResult);
        }

        const auto& weightIsNan = [](const WeightedRelationItem & item) {
          return std::isnan(item.getWeight());
        };

        // Erase all items with a weight of NAN
        weightedRelations.erase(std::remove_if(weightedRelations.begin(),
                                               weightedRelations.end(),
                                               weightIsNan),
                                weightedRelations.end());

        std::sort(weightedRelations.begin(), weightedRelations.end());
      }

    private:
      ChooseableFilter<AFilterFactory> m_filter;
    };
  }
}
