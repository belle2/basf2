/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/findlets/LimitedOnHitApplier.dcl.h>
#include <tracking/vxdHoughTracking/findlets/PathLengthToggledApplier.dcl.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/PathFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  namespace vxdHoughTracking {
    class VXDHoughState;

    // Guard to prevent repeated instantiations
    extern template class
    PathLengthToggledApplier<VXDHoughState, LimitedOnHitApplier<VXDHoughState, TrackFindingCDC::ChooseableFilter<PathFilterFactory>>>;

                             /// Alias to apply the () operator to all items filtered by CKF SVD layer states
                             using ChooseablePathFilter =
                             PathLengthToggledApplier<VXDHoughState, LimitedOnHitApplier<VXDHoughState, TrackFindingCDC::ChooseableFilter<PathFilterFactory>>>;

  }
}
