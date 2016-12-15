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
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    template <class ACollectorItem, class ACollectionItem>
    class CutSelector :
      public Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      using WeightedRelationItem = WeightedRelation<ACollectorItem, const ACollectionItem>;

      /// The parent class
      using Super = Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>>;

      void exposeParameters(ModuleParamList* moduleParamList,
                            const std::string& prefix) override
      {
        Super::exposeParameters(moduleParamList, prefix);

        moduleParamList->addParameter(prefixed(prefix, "cutValue"), m_param_cutValue,
                                      "Value to cut at.",
                                      m_param_cutValue);
      }

      void apply(std::vector<WeightedRelationItem>& weightedRelations) override
      {
        const auto& weightIsNan = [](const WeightedRelationItem & item) {
          return std::isnan(item.getWeight());
        };

        // Erase all items with a weight of NAN
        weightedRelations.erase(std::remove_if(weightedRelations.begin(),
                                               weightedRelations.end(),
                                               weightIsNan),
                                weightedRelations.end());

        // As the vector is sorted, we just have to find the first occurring element below the cut value
        const auto& lessThanCut = [this](const WeightedRelationItem & relationItem) {
          return relationItem.getWeight() < m_param_cutValue;
        };

        weightedRelations.erase(std::remove_if(weightedRelations.begin(),
                                               weightedRelations.end(),
                                               lessThanCut),
                                weightedRelations.end());

        std::sort(weightedRelations.begin(), weightedRelations.end());
      }

      void setCutValue(Weight cutValue)
      {
        m_param_cutValue = cutValue;
      }

    private:
      Weight m_param_cutValue = NAN;
    };
  }
}
