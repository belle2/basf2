/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/datcon/optimizedDATCON/findlets/RelationCreator.dcl.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

namespace Belle2 {
  template<class AState, class ARelationFilter>
  RelationCreator<AState, ARelationFilter>::~RelationCreator() = default;

  template<class AState, class ARelationFilter>
  RelationCreator<AState, ARelationFilter>::RelationCreator() : Super()
  {
    Super::addProcessingSignalListener(&m_relationFilter);
  }

  template<class AState, class ARelationFilter>
  void RelationCreator<AState, ARelationFilter>::exposeParameters(ModuleParamList* moduleParamList,
      const std::string& prefix)
  {
    m_relationFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("twoHitRelation", prefix));
  }

  template<class AState, class ARelationFilter>
  void RelationCreator<AState, ARelationFilter>::apply(std::vector<AState*>& states,
                                                       std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations)
  {
    // relations += states -> states
    TrackFindingCDC::RelationFilterUtil::appendUsing(m_relationFilter, states, states, relations, 100000);
  }
}
