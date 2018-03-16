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
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <tracking/trackFindingCDC/utilities/Range.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  /// Base filter for CKF PXD states
  template <class AFilter>
  class LayerPXDRelationFilter : public TrackFindingCDC::RelationFilter<CKFToPXDState> {
    /// The parent class
    using Super = TrackFindingCDC::RelationFilter<CKFToPXDState>;

  public:
    using Super::operator();

    /// Add the filter as listener
    LayerPXDRelationFilter();

    /// Default destructor
    ~LayerPXDRelationFilter();

    std::vector<CKFToPXDState*> getPossibleTos(CKFToPXDState* from,
                                               const std::vector<CKFToPXDState*>& states) const override;

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    TrackFindingCDC::Weight operator()(const CKFToPXDState& from, const CKFToPXDState& to) override;

  private:
    /// Parameter: Make it possible to jump over N layers.
    int m_param_hitJumping = 0;
    /// Filter for rejecting the states
    AFilter m_filter;
  };
}
