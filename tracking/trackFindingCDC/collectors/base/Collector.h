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
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    template<class ACollectorItem, class ACollectionItem>
    class Collector {
    public:
      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "");
      typedef ACollectorItem CollectorItem;
      typedef ACollectionItem CollectionItem;

    public:
      virtual ~Collector() { }

      virtual void initialize() { }

      virtual void terminate() { }

      void collect(std::vector<ACollectorItem>& collectorList, const std::vector<ACollectionItem>& collectionItems)
      {
        std::map<const ACollectionItem*, std::vector<std::pair<ACollectorItem*, Weight>>> matches;

        for (ACollectorItem& collectorItem : collectorList) {
          const std::vector<std::pair<const ACollectionItem*, Weight>>& matchesForCollector = match(collectorItem, collectionItems);

          for (const auto& matchedCollectionItemWithWeight : matchesForCollector) {
            const ACollectionItem* collectionItem = matchedCollectionItemWithWeight.first;
            const Weight weight = matchedCollectionItemWithWeight.second;

            if (matches.find(collectionItem) != matches.end()) {
              matches[collectionItem].emplace_back(&collectorItem, weight);
            } else {
              std::vector<std::pair<ACollectorItem*, Weight>> collectorItemsWithWeights;
              collectorItemsWithWeights.reserve(5);
              collectorItemsWithWeights.emplace_back(&collectorItem, weight);
              matches.emplace(collectionItem, collectorItemsWithWeights);
            }
          }
        }

        std::map<ACollectorItem*, std::vector<const ACollectionItem*>> filteredMatches;
        for (auto& matchedCollectorItemsWithWeights : matches) {
          const ACollectionItem* collectionItem = matchedCollectorItemsWithWeights.first;
          std::vector<std::pair<ACollectorItem*, Weight>>& collectorItemsWithWeights = matchedCollectorItemsWithWeights.second;

          auto maximalElement = std::max_element(collectorItemsWithWeights.begin(),
          collectorItemsWithWeights.end(), [](const std::pair<ACollectorItem*, Weight>& lhs, const std::pair<ACollectorItem*, Weight>& rhs) {
            return lhs.second < rhs.second;
          });

          ACollectorItem* bestCollectorItem = maximalElement->first;

          if (filteredMatches.find(bestCollectorItem) != filteredMatches.end()) {
            filteredMatches[bestCollectorItem].emplace_back(collectionItem);
          } else {
            std::vector<const ACollectionItem*> collectionItems;
            collectionItems.reserve(50);
            collectionItems.emplace_back(collectionItem);
            filteredMatches.emplace(bestCollectorItem, collectionItems);
          }
        }

        for (auto& collectorItemWithMatches : filteredMatches) {
          ACollectorItem* collectorItem = collectorItemWithMatches.first;
          const std::vector<const ACollectionItem*>& matchedCollectionItems = collectorItemWithMatches.second;

          add(*collectorItem, matchedCollectionItems);
        }
      }

    protected:
      virtual std::vector<std::pair<const ACollectionItem*, Weight>> match(const ACollectorItem& collectorItem,
                                                                  const std::vector<ACollectionItem>& collectionList) = 0;
      virtual void add(ACollectorItem& collectorItem, const std::vector<const ACollectionItem*>& matchedItems) = 0;
    };
  }
}
