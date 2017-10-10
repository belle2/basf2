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
  class LayerSVDRelationFilter : public TrackFindingCDC::RelationFilter<CKFToSVDState> {
  public:
    /// Default constructor
    LayerSVDRelationFilter();

    /// Default destructor
    ~LayerSVDRelationFilter();

    std::vector<CKFToSVDState*> getPossibleTos(CKFToSVDState* from,
                                               const std::vector<CKFToSVDState*>& states) const override;

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Parameter: Make it possible to jump over N layers.
    int m_param_hitJumping = 1;
  };
}
