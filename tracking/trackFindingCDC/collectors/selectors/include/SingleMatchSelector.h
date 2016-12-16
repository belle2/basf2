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
    template <class ACollectorItem, class ACollectionItem, class AComparer = std::less<const ACollectionItem*>>
    class SingleMatchSelector :
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

        moduleParamList->addParameter(prefixed(prefix, "useOnlySingleBestCandidate"), m_param_useOnlySingleBestCandidate,
                                      "Use only the found candidate, if it is the only one. Otherwise, use the best one.",
                                      m_param_useOnlySingleBestCandidate);
      }

      void apply(std::vector<WeightedRelationItem>& weightedRelations) override
      {
        B2ASSERT("The relations need to be sorted for this selector!",
                 std::is_sorted(weightedRelations.begin(), weightedRelations.end()));

        // Build a map from collectionItem -> matched collectionItems with weight
        std::map<const ACollectionItem*, std::vector<WeightedRelationItem>, AComparer> collectionItemToMatchedMap;

        for (const WeightedRelationItem& relation : weightedRelations) {
          collectionItemToMatchedMap[relation.getTo()].push_back(relation);
        }

        // Clear all relations and start filling in only the ones which are valid
        weightedRelations.clear();

        for (const auto& collectionItemToMatches : collectionItemToMatchedMap) {
          const auto& matches = collectionItemToMatches.second;

          if (not m_param_useOnlySingleBestCandidate or matches.size() == 1) {
            const auto& bestMatch = std::min_element(matches.begin(), matches.end(), GreaterWeight());
            weightedRelations.push_back(*bestMatch);
          }
        }

        std::sort(weightedRelations.begin(), weightedRelations.end());
      }

      void setUseOnlySingleBestCandidate(bool useOnlySingleBestCandidate)
      {
        m_param_useOnlySingleBestCandidate = useOnlySingleBestCandidate;
      }

    private:
      bool m_param_useOnlySingleBestCandidate = true;
    };
  }
}
