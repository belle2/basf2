/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>

#include <array>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Relation filter the creation of relations
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

      /// Get the weight of the relation between from and to
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
