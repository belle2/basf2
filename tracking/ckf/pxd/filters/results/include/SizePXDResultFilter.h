/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.dcl.h>
#include <tracking/ckf/pxd/filters/results/PXDResultVarSet.h>

namespace Belle2 {
  /// Base filter for CKF PXD results (on overlap check)
  class SizePXDResultFilter : public TrackFindingCDC::ChoosableFromVarSetFilter<PXDResultVarSet> {
  public:
    SizePXDResultFilter() : TrackFindingCDC::ChoosableFromVarSetFilter<PXDResultVarSet>("number_of_hits")
    {

    }
  };
}
