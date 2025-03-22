/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>
#include <tracking/trackingUtilities/filters/base/RelationFilterUtil.h>
#include <tracking/trackingUtilities/utilities/WeightedRelation.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Findlet for applying filters for creating hit-hit relations in each track candidate
    template<class AHit, class ARelationFilter>
    class RelationCreator : public TrackingUtilities::Findlet<AHit*, TrackingUtilities::WeightedRelation<AHit>> {
    public:
      /// The parent class
      using Super = TrackingUtilities::Findlet<AHit*, TrackingUtilities::WeightedRelation<AHit>>;

      /// Construct this findlet and add the subfindlet as listener
      RelationCreator()
      {
        Super::addProcessingSignalListener(&m_relationFilter);
      };

      /// Default destructor
      ~RelationCreator() = default;

      /// Expose the parameters of the subfindlet
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final
      {
        m_relationFilter.exposeParameters(moduleParamList, TrackingUtilities::prefixed("relation", prefix));
      };

      /// Apply both filters for creating state-hit and hit-hit relations
      void apply(std::vector<AHit*>& hits,
                 std::vector<TrackingUtilities::WeightedRelation<AHit>>& relations) override
      {
        // relations += hits -> hits in each track candidate
        TrackingUtilities::RelationFilterUtil::appendUsing(m_relationFilter, hits, hits, relations, 100000);
      };

    private:
      /// Subfindlet for the relation checking between seed and hits
      ARelationFilter m_relationFilter;
    };

  }
}
