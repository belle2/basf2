/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>

#include <string>
#include <map>
#include <array>
#include <vector>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCWireHit;

    /**
     *  Wire hit relation filter that is compensating for hit inefficiencies.
     *
     *  The default setup counters some hits lost due to a drift time cut off
     *  if the track enters at the edge of a drift cell. The biggest effect observed in the CDC simulation
     *  was observed when the track is along the 2, 4, 8 and 10 o'clock direction since in this case
     *  *two* neighboring hits can be lost due to this time cut off.
     *  In an attempt to detect if this occured the primary wire hit neighborhood
     *  is slightly extended to bridge to the secondary neighborhood in the critical directions.
     *
     *  The criterion can be lowered such that fewer missing hits trigger the inclusion of the
     *  secondary neighbors.
     */
    class BridgingWireHitRelationFilter : public RelationFilter<CDCWireHit> {

    private:
      /// Type of the base class
      using Super = RelationFilter<CDCWireHit>;

    public:
      /// Default constructor
      BridgingWireHitRelationFilter();

      /// Default destructor
      ~BridgingWireHitRelationFilter();

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Receive signal at the begin of the event processing and prepare some parameters
      void initialize() override;

      /**
       *  Returns a vector containing the neighboring wire hits of the given wire hit out of the
       *  sorted range given by the two iterator other argumets.
       */
      std::vector<CDCWireHit*> getPossibleTos(CDCWireHit* from,
                                              const std::vector<CDCWireHit*>& wireHits) const final;

    private:
      /// Parameter: A map from o'clock direction to the number of missing primary drift cells
      std::map<int, int> m_param_missingPrimaryNeighborThresholdMap =
      {{0, 2}, {2, 2}, {4, 2}, {6, 2}, {8, 2}, {10, 2}};

      /// Array for the number of primary drift cells to be included for the o'clock position at each index.
      std::array<short, 12> m_missingPrimaryNeighborThresholds;

      /// Indices of the considered o'clock positions of the secondary neighborhood.
      std::vector<short> m_consideredSecondaryNeighbors;
    };
  }
}
