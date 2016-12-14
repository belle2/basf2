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
      public Findlet<WeightedRelation<const ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      typedef WeightedRelation<const ACollectorItem, const ACollectionItem> WeightedRelationItem;

      /// The parent class
      typedef Findlet<WeightedRelation<const ACollectorItem, const ACollectionItem>> Super;

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
        B2ASSERT("The relations need to be sorted in non-ascending order for this selector!",
                 std::is_sorted(weightedRelations.begin(), weightedRelations.end(), WeightedRelationsGreater<WeightedRelationItem>()));

        // As the vector is sorted, we just have to find the first occurring element below the cut value
        const auto& lessThanCut = [this](const WeightedRelationItem & relationItem) {
          return relationItem.getWeight() < m_param_cutValue;
        };

        auto firstElementLessThanCut = std::find_if(weightedRelations.begin(), weightedRelations.end(), lessThanCut);
        weightedRelations.erase(firstElementLessThanCut, weightedRelations.end());
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
