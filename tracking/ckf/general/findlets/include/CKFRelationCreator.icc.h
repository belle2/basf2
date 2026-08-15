/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/CKFRelationCreator.dcl.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>

#include <tracking/trackingUtilities/filters/base/RelationFilterUtil.h>
#include <tracking/trackingUtilities/utilities/Algorithms.h>

#include <algorithm>

namespace Belle2 {
  template<class AState, class ASeedRelationFilter, class AHitRelationFilter>
  CKFRelationCreator<AState, ASeedRelationFilter, AHitRelationFilter>::~CKFRelationCreator() = default;

  template<class AState, class ASeedRelationFilter, class AHitRelationFilter>
  CKFRelationCreator<AState, ASeedRelationFilter, AHitRelationFilter>::CKFRelationCreator() : Super()
  {
    Super::addProcessingSignalListener(&m_seedFilter);
    Super::addProcessingSignalListener(&m_hitFilter);
  }

  template<class AState, class ASeedRelationFilter, class AHitRelationFilter>
  void CKFRelationCreator<AState, ASeedRelationFilter, AHitRelationFilter>::exposeParameters(ModuleParamList* moduleParamList,
      const std::string& prefix)
  {
    m_seedFilter.exposeParameters(moduleParamList, TrackingUtilities::prefixed("seed", prefix));
    m_hitFilter.exposeParameters(moduleParamList, TrackingUtilities::prefixed("hit", prefix));

    moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "onlyUseHitStatesRelatedToSeeds"),
                                  m_onlyUseHitStatesRelatedToSeeds,
                                  "Only use hit states related to seed states to build the inter hit relations to reduce combinatorics. "\
                                  "By default, only the \"FromStates\" will be the ones related to seeds. If also the \"ToStates\" should be "\
                                  "related to the seeds, also m_onlyCombineRelatedHitStates (name in Python: " + \
                                  TrackingUtilities::prefixed(prefix, "onlyCombineRelatedHitStates") + ") should be set to true.",
                                  m_onlyUseHitStatesRelatedToSeeds);
    moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "onlyCombineRelatedHitStates"),
                                  m_onlyCombineRelatedHitStates,
                                  "Only use hit states related to seed states to build the inter hit relations to reduce combinatorics. "\
                                  "If true, both \"FromStates\" and \"ToStates\" will be those that are already related to seeds. "\
                                  "Only works if also m_onlyUseHitStatesRelatedToSeeds (name in Python: " + \
                                  TrackingUtilities::prefixed(prefix, "onlyUseHitStatesRelatedToSeeds") + ") is set to true.",
                                  m_onlyCombineRelatedHitStates);
  }

  template<class AState, class ASeedRelationFilter, class AHitRelationFilter>
  void CKFRelationCreator<AState, ASeedRelationFilter, AHitRelationFilter>::apply(std::vector<AState>& seedStates,
      std::vector<AState>& states,
      std::vector<TrackingUtilities::WeightedRelation<AState>>& relations)
  {
    const std::vector<AState*> seedStatePointers = TrackingUtilities::as_pointers<AState>(seedStates);
    const std::vector<AState*> statePointers = TrackingUtilities::as_pointers<AState>(states);

    // Just some arbitrary number...
    relations.reserve(10000);

    // relations += seed states -> states
    TrackingUtilities::RelationFilterUtil::appendUsing(m_seedFilter, seedStatePointers, statePointers, relations, 1000000);

    // relations += states -> states
    if (m_onlyUseHitStatesRelatedToSeeds) {
      // only use subset of hit states for inter hit state relation creation
      std::vector<AState*> selectedStatePointers;
      selectedStatePointers.reserve(relations.size());

      for (const auto& relation : relations) {
        // hit state pointers are the "To"s in the relation, only take those
        const auto it = std::find(selectedStatePointers.begin(), selectedStatePointers.end(), relation.getTo());
        if (it == selectedStatePointers.end()) {
          selectedStatePointers.push_back(relation.getTo());
        }
      }

      if (m_onlyCombineRelatedHitStates) {
        // Reduce combinatorics a lot by only combining selectedStatePointers with other selectedStatePointers
        TrackingUtilities::RelationFilterUtil::appendUsing(m_hitFilter, selectedStatePointers, selectedStatePointers, relations, 1000000);
      } else {
        // Reduce combinatorics a bit less by only combining selectedStatePointers essentially all statePointers
        TrackingUtilities::RelationFilterUtil::appendUsing(m_hitFilter, selectedStatePointers, statePointers, relations, 1000000);
      }
    } else {
      // use all of hit states for inter hit state relation creation
      TrackingUtilities::RelationFilterUtil::appendUsing(m_hitFilter, statePointers, statePointers, relations, 1000000);
    }
  }
}
