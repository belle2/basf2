/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <tracking/trackFindingCDC/utilities/Range.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  /// Base filter for CKF SVD states
  template <class AFilter>
  class LayerSVDRelationFilter : public TrackFindingCDC::RelationFilter<CKFToSVDState> {
    /// The parent class
    using Super = TrackFindingCDC::RelationFilter<CKFToSVDState>;

  public:
    using Super::operator();

    /// Add the filter as listener
    LayerSVDRelationFilter();

    /// Default destructor
    ~LayerSVDRelationFilter();

    /// Return all states the given state is possible related to.
    std::vector<CKFToSVDState*> getPossibleTos(CKFToSVDState* from,
                                               const std::vector<CKFToSVDState*>& states) const final;

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;


    /// Give a final weight to the possibilities by asking the filter.
    TrackFindingCDC::Weight operator()(const CKFToSVDState& from, const CKFToSVDState& to) final;

    /// Initialize the maximal ladder cache
    void beginRun() final;

  private:
    /// Parameter: Make it possible to jump over N layers.
    int m_param_hitJumping = 1;
    /// Filter for rejecting the states
    AFilter m_filter;
    /// Cached number of ladders per layer
    std::map<short, unsigned long> m_maximalLadderCache;
  };
}
