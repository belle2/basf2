/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    // relations += seed states -> states
    TrackFindingCDC::RelationFilterUtil::appendUsing(m_seedFilter, seedStatePointers, statePointers, relations);

    // relations += states -> states
    TrackFindingCDC::RelationFilterUtil::appendUsing(m_hitFilter, statePointers, statePointers, relations);
  }
}
