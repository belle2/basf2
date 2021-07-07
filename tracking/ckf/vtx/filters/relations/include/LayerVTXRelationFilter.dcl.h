/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>
#include <tracking/ckf/vtx/entities/CKFToVTXState.h>

namespace Belle2 {
  /// Base filter for CKF VTX states
  template <class AFilter, class APrefilter = AFilter>
  class LayerVTXRelationFilter : public TrackFindingCDC::RelationFilter<CKFToVTXState> {
    /// The parent class
    using Super = TrackFindingCDC::RelationFilter<CKFToVTXState>;

  public:
    using Super::operator();

    /// Add the filter as listener
    LayerVTXRelationFilter();

    /// Default destructor
    ~LayerVTXRelationFilter();

    /// Return all states the given state is possible related to.
    std::vector<CKFToVTXState*> getPossibleTos(CKFToVTXState* from,
                                               const std::vector<CKFToVTXState*>& states) const override;

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Give a final weight to the possibilities by asking the filter.
    TrackFindingCDC::Weight operator()(const CKFToVTXState& from, const CKFToVTXState& to) override;

    /// Initialize the maximal ladder cache
    void beginRun() final;

  private:
    /// Parameter: Make it possible to jump over N layers.
    int m_param_hitJumping = 1;
    /// Filter for rejecting the states
    AFilter m_filter;
    /// Loose pre-filter to reject possibleTos
    APrefilter m_prefilter;
    /// Cached number of ladders per layer
    std::map<short, unsigned long> m_maximalLadderCache;
  };
}
