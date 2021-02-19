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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /// Findlet for applying filters for creating hit-hit and hit-seed relations
  template<class AState, class ARelationFilter>
  class RelationCreator : public TrackFindingCDC::Findlet<AState*, TrackFindingCDC::WeightedRelation<AState>> {
  public:
    /// The parent class
    using Super = TrackFindingCDC::Findlet<AState*, TrackFindingCDC::WeightedRelation<AState>>;

    /// Construct this findlet and add the subfindlet as listener
    RelationCreator()
    {
      Super::addProcessingSignalListener(&m_relationFilter);
    };

    /// Default destructor
    ~RelationCreator() = default;

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
      m_relationFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("twoHitRelation", prefix));
    };

    /// Apply both filters for creating state-hit and hit-hit relations
    void apply(std::vector<AState*>& states,
               std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations) override
    {
      // relations += states -> states
      TrackFindingCDC::RelationFilterUtil::appendUsing(m_relationFilter, states, states, relations, 100000);
    };

  private:
    /// Subfindlet for the relation checking between seed and hits
    ARelationFilter m_relationFilter;
  };
}
