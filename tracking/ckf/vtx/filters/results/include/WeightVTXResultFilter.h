/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.dcl.h>
#include <tracking/ckf/vtx/filters/results/VTXResultVarSet.h>
#include <tracking/ckf/vtx/entities/CKFToVTXResult.h>

namespace Belle2 {
  /// Base filter for CKF VTX results (on overlap check)
  class WeightVTXResultFilter : public BaseVTXResultFilter {
  public:
    /// Return the sum of CKF VTX weights
    TrackFindingCDC::Weight operator()(const CKFToVTXResult& result) override
    {
      return result.getWeightSum();
    }
  };
}
