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

#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <vector>
#include <string>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Selector to remove all weighted relations with a weight below a certain cut value.
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
    class CutSelector :
      public Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      using WeightedRelationItem = WeightedRelation<ACollectorItem, const ACollectionItem>;

      /// The parent class
      using Super = Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>>;

      /// Expose the cut value to the module.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        Super::exposeParameters(moduleParamList, prefix);
        moduleParamList->addParameter(prefixed(prefix, "cutValue"), m_param_cutValue,
                                      "Value to cut at.",
                                      m_param_cutValue);
      }

      /// Do the cut.
      void apply(std::vector<WeightedRelationItem>& weightedRelations) override
      {
        const auto& lessThanCutOrNaN = [this](const WeightedRelationItem & relationItem) {
          return std::isnan(relationItem.getWeight()) or relationItem.getWeight() < m_param_cutValue;
        };

        // Erase all items with a weight of NAN
        weightedRelations.erase(std::remove_if(weightedRelations.begin(),
                                               weightedRelations.end(),
                                               lessThanCutOrNaN),
                                weightedRelations.end());
      }

      /// Function to set the cut value (mostly for tests).
      void setCutValue(Weight cutValue)
      {
        m_param_cutValue = cutValue;
      }

    private:
      /// The cut value to use.
      Weight m_param_cutValue = NAN;
    };
  }
}
