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

#include <tracking/ckf/svd/filters/relations/LayerSVDRelationFilter.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

namespace Belle2 {
  /// Base filter for CKF SVD states
  class SectorSVDRelationFilter : public LayerSVDRelationFilter {
  public:
    TrackFindingCDC::Weight operator()(const CKFToSVDState& from, const CKFToSVDState& to) override;
  };
}
