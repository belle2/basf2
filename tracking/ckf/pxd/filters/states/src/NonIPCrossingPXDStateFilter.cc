/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/states/NonIPCrossingPXDStateFilter.h>
#include <tracking/ckf/general/filters/NonIPCrossingStateFilter.icc.h>

namespace Belle2 {
  template class NonIPCrossingStateFilter<AllPXDStateFilter>;
}