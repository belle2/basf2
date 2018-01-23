/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/states/NonIPCrossingPXDStateFilter.h>
#include <tracking/ckf/general/filters/NonIPCrossingStateFilter.icc.h>

namespace Belle2 {
  template class NonIPCrossingStateFilter<AllPXDStateFilter>;
}