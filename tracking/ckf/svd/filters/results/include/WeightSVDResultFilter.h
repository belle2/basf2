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
#include <tracking/ckf/svd/filters/results/SVDResultVarSet.h>
#include <tracking/ckf/svd/entities/CKFToSVDResult.h>

namespace Belle2 {
  /// Base filter for CKF SVD results (on overlap check)
  class WeightSVDResultFilter : public TrackFindingCDC::ChoosableFromVarSetFilter<SVDResultVarSet> {
  public:
    WeightSVDResultFilter() : TrackFindingCDC::ChoosableFromVarSetFilter<SVDResultVarSet>("weight_sum")
    {

    }
  };
}
