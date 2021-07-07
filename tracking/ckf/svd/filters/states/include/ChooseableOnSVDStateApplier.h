/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
