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
    TrackFindingCDC::RelationFilterUtil::appendUsing(m_hitFilter, statePointers, statePointers, relations, 1000000);
  }
}
