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

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>

#include <array>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Base filter for hits in VXDHoughState
    template <class AFilter>
    class LayerRelationFilter : public TrackFindingCDC::RelationFilter<VXDHoughState> {
      /// The parent class
      using Super = TrackFindingCDC::RelationFilter<VXDHoughState>;

    public:
      using Super::operator();

      /// Add the filter as listener
      LayerRelationFilter();

      /// Default destructor
      ~LayerRelationFilter();

      /// Return all states the given state is possible related to.
      std::vector<VXDHoughState*> getPossibleTos(VXDHoughState* from, const std::vector<VXDHoughState*>& states) const final;

      /// Expose the parameters of the filter
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      TrackFindingCDC::Weight operator()(const VXDHoughState& from, const VXDHoughState& to) final;

      /// Initialize the maximal ladder cache
      void beginRun() final;

    private:
      /// Parameter: Make it possible to jump over N layers.
      int m_param_hitJumping = 1;
      /// Filter for rejecting the states
      AFilter m_filter;
      /// Cached number of ladders per layer
      std::array<ushort, 7> m_maximalLadderCache;
    };

  }
}
