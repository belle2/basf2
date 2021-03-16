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

#include <tracking/datcon/optimizedDATCON/findlets/LimitedOnHitApplier.dcl.h>
#include <tracking/datcon/optimizedDATCON/findlets/PathLengthToggledApplier.dcl.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/PathFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  class HitData;
  /// A chooseable filter for picking out the relations between states
//   using ChooseablePathFilter = TrackFindingCDC::ChooseableFilter<PathFilterFactory>;

  // Guard to prevent repeated instantiations
  extern template class PathLengthToggledApplier<HitData, LimitedOnHitApplier<HitData,
                                                 TrackFindingCDC::ChooseableFilter<PathFilterFactory>>>;
                                                 /// Alias to apply the () operator to all items filtered by CKF SVD layer states
                                                 using ChooseablePathFilter = PathLengthToggledApplier<HitData, LimitedOnHitApplier<HitData,
                                                     TrackFindingCDC::ChooseableFilter<PathFilterFactory>>>;

}
