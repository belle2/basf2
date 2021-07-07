/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/svd/filters/states/AllSVDStateFilter.h>
#include <tracking/ckf/general/filters/NonIPCrossingStateFilter.dcl.h>

namespace Belle2 {
  extern template class NonIPCrossingStateFilter<AllSVDStateFilter>;
  /// Alias for filter to check direction of a new CKF SVD state
  using NonIPCrossingSVDStateFilter = NonIPCrossingStateFilter<AllSVDStateFilter>;
}
