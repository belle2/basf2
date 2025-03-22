/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/filters/base/ChoosableFromVarSetFilter.dcl.h>
#include <tracking/ckf/pxd/filters/results/PXDResultVarSet.h>

namespace Belle2 {
  /// Base filter for CKF PXD results (on overlap check)
  class SizePXDResultFilter : public TrackingUtilities::ChoosableFromVarSetFilter<PXDResultVarSet> {
  public:
    SizePXDResultFilter() : TrackingUtilities::ChoosableFromVarSetFilter<PXDResultVarSet>("number_of_hits")
    {

    }
  };
}
