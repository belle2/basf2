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

#include <tracking/ckf/svd/filters/relations/BaseSVDPairFilter.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

namespace Belle2 {
  /// Base filter for CKF SVD states
  class SensorSVDPairFilter : public BaseSVDPairFilter {
  public:
    /// Return the weight based on layer
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation) override;
  };
}
