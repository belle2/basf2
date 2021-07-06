/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.dcl.h>
#include <tracking/ckf/svd/filters/results/SVDResultVarSet.h>
#include <tracking/ckf/svd/entities/CKFToSVDResult.h>

namespace Belle2 {
  /// Base filter for CKF SVD results (on overlap check)
  class WeightSVDResultFilter : public BaseSVDResultFilter {
  public:
    /// Return the sum of CKF SVD weights
    TrackFindingCDC::Weight operator()(const CKFToSVDResult& result) override
    {
      return result.getWeightSum();
    }
  };
}
