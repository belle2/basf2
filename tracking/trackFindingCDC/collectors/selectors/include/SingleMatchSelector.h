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

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Selector to remove all relations in the list, which share the same collection item - except one in case the
     * useOnlySingleBestCandidate is set to false (then, only the one with the heightest weight is used).
     *
     * This means the function will output a list of weighted relations, where the relations from collection to
     * collector items are injective (but not in the other way round: a collector can have multiple matched
     * collection items, but one collection item only one matched collector item).
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
    template <class ACollectorItem, class ACollectionItem, class AComparer = std::less<const ACollectionItem*>>
    class SingleMatchSelector :
      public Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      using WeightedRelationItem = WeightedRelation<ACollectorItem, const ACollectionItem>;

      /// The parent class
      using Super = Findlet<WeightedRelation<ACollectorItem, const ACollectionItem>>;

      /// Expose the useOnlySingleBestCandidate parameter to the module.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        Super::exposeParameters(moduleParamList, prefix);

        moduleParamList->addParameter(prefixed(prefix, "useOnlySingleBestCandidate"), m_param_useOnlySingleBestCandidate,
                                      "Use only the found candidate, if it is the only one. Otherwise, use the best one.",
                                      m_param_useOnlySingleBestCandidate);
      }

      /// Main function of this class doing the relation selection.
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

          const auto& bestMatch = std::min_element(matches.begin(), matches.end(), GreaterWeight());

          const bool addBestMatch = matches.size() == 1 or not m_param_useOnlySingleBestCandidate;

          if (addBestMatch) {
            weightedRelations.push_back(*bestMatch);
          }
        }

        std::sort(weightedRelations.begin(), weightedRelations.end());
      }

      /// Set the UseOnlySingleBestCandidate parameter (mostly for tests).
      void setUseOnlySingleBestCandidate(bool useOnlySingleBestCandidate)
      {
        m_param_useOnlySingleBestCandidate = useOnlySingleBestCandidate;
      }

    private:
      /**
       * Whether to output only those relations, where the relation from collectio to collector
       * items is injective or to output always the best relation for a given collection item.
       */
      bool m_param_useOnlySingleBestCandidate = true;
    };
  }
}
