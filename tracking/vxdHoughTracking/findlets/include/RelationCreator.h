/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Findlet for applying filters for creating hit-hit relations in each track candidate
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
      };

      /// Apply both filters for creating state-hit and hit-hit relations
      void apply(std::vector<AHit*>& hits,
                 std::vector<TrackFindingCDC::WeightedRelation<AHit>>& relations) override
      {
        // relations += hits -> hits in each track candidate
        TrackFindingCDC::RelationFilterUtil::appendUsing(m_relationFilter, hits, hits, relations, 100000);
      };

    private:
      /// Subfindlet for the relation checking between seed and hits
      ARelationFilter m_relationFilter;
    };

  }
}
