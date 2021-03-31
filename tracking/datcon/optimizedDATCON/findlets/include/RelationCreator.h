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
  template<class AHit, class ARelationFilter>
  class RelationCreator : public TrackFindingCDC::Findlet<AHit*, TrackFindingCDC::WeightedRelation<AHit>> {
  public:
    /// The parent class
    using Super = TrackFindingCDC::Findlet<AHit*, TrackFindingCDC::WeightedRelation<AHit>>;

    /// Construct this findlet and add the subfindlet as listener
    RelationCreator()
    {
      Super::addProcessingSignalListener(&m_relationFilter);
    };

    /// Default destructor
    ~RelationCreator() = default;

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
      m_relationFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("relation", prefix));

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maxRelations"), m_maxRelations,
      "Maximum number of relations to be created.",  m_maxRelations);
    };

    /// Apply both filters for creating state-hit and hit-hit relations
    void apply(std::vector<AHit*>& hits,
               std::vector<TrackFindingCDC::WeightedRelation<AHit>>& relations) override
    {
      // relations += hits -> hits
      TrackFindingCDC::RelationFilterUtil::appendUsing(m_relationFilter, hits, hits, relations, m_maxRelations);
    };

  private:
    /// Subfindlet for the relation checking between seed and hits
    ARelationFilter m_relationFilter;

    /// maximum number of relations that can be created per track candidate
    uint m_maxRelations = 10000;
  };
}
