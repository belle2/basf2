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

#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.dcl.h>
#include <tracking/ckf/pxd/filters/results/PXDResultVarSet.h>
#include <tracking/ckf/pxd/entities/CKFToPXDResult.h>

namespace Belle2 {
  /// Base filter for CKF PXD results (on overlap check)
  class SizePXDResultFilter : public TrackFindingCDC::ChoosableFromVarSetFilter<PXDResultVarSet> {
  public:
    SizePXDResultFilter() : TrackFindingCDC::ChoosableFromVarSetFilter<PXDResultVarSet>("number_of_hits")
    {

    }
  };
}
