/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/svd/filters/states/AllSVDStateFilter.h>
#include <tracking/ckf/general/filters/NonIPCrossingStateFilter.dcl.h>

namespace Belle2 {
  extern template class NonIPCrossingStateFilter<AllSVDStateFilter>;
  /// Alias for filter to check direction of a new CKF SVD state
  using NonIPCrossingSVDStateFilter = NonIPCrossingStateFilter<AllSVDStateFilter>;
}
