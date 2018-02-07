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

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  /// Base filter for CKF SVD states
  using BaseSVDStateFilter =
    TrackFindingCDC::Filter<std::pair<const std::vector<TrackFindingCDC::WithWeight<const CKFToSVDState*>>, CKFToSVDState*>>;
}
