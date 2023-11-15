/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/CKFRelationCreator.dcl.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

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
    m_seedFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("seed", prefix));
    m_hitFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("hit", prefix));

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "onlyUseHitStatesRelatedToSeeds"),
                                  m_onlyUseHitStatesRelatedToSeeds,
                                  "Only use hit states related to seed states to build the inter hit relations to reduce combinatorics.",
                                  m_onlyUseHitStatesRelatedToSeeds);
  }

  template<class AState, class ASeedRelationFilter, class AHitRelationFilter>
  void CKFRelationCreator<AState, ASeedRelationFilter, AHitRelationFilter>::apply(std::vector<AState>& seedStates,
      std::vector<AState>& states,
      std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations)
  {
    const std::vector<AState*> seedStatePointers = TrackFindingCDC::as_pointers<AState>(seedStates);
    const std::vector<AState*> statePointers = TrackFindingCDC::as_pointers<AState>(states);

    // Just some arbitrary number...
    relations.reserve(10000);

    // relations += seed states -> states
    TrackFindingCDC::RelationFilterUtil::appendUsing(m_seedFilter, seedStatePointers, statePointers, relations, 1000000);

    // relations += states -> states
    if (m_onlyUseHitStatesRelatedToSeeds) {
      // only use subset of hit states for inter hit state relation creation
      std::vector<AState*> selectedStatePointers;
      selectedStatePointers.reserve(relations.size());

      for (const auto relation : relations) {
        // hit state pointers are the "To"s in the relation, only take those
        selectedStatePointers.push_back(relation.getTo());
      }
      TrackFindingCDC::RelationFilterUtil::appendUsing(m_hitFilter, selectedStatePointers, selectedStatePointers, relations, 1000000);
    } else {
      // use all of hit states for inter hit state relation creation
      TrackFindingCDC::RelationFilterUtil::appendUsing(m_hitFilter, statePointers, statePointers, relations, 1000000);
    }
  }
}
