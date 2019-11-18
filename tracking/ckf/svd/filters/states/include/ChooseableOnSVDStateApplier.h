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

#include <tracking/ckf/general/findlets/LimitedOnStateApplier.dcl.h>
#include <tracking/ckf/general/findlets/LayerToggledApplier.dcl.h>
#include <tracking/ckf/svd/filters/states/SVDStateFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  class CKFToSVDState;
  // Guard to prevent repeated instantiations
  extern template class LayerToggledApplier<CKFToSVDState, LimitedOnStateApplier<CKFToSVDState,
                                            TrackFindingCDC::ChooseableFilter<SVDStateFilterFactory>>>;
                                            /// Alias to apply the () operator to all items filtered by CKF SVD layer states
                                            using ChooseableOnSVDStateApplier = LayerToggledApplier<CKFToSVDState, LimitedOnStateApplier<CKFToSVDState,
                                                TrackFindingCDC::ChooseableFilter<SVDStateFilterFactory>>>;
}
