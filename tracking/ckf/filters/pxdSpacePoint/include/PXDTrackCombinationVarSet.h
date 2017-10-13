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

#include <tracking/ckf/filters/cdcToSpacePoint/result/BaseVXDTrackCombinationFilter.h>
#include <tracking/ckf/findlets/base/AdvanceAlgorithm.h>
#include <tracking/ckf/findlets/base/KalmanUpdateFitter.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const pxdTrackCombinationNames[] = {
    "seed_svd_hits",
    "seed_lowest_svd_layer",
    "seed_cdc_hits",
    "seed_lowest_cdc_layer",
  };

  /// Vehicle class to transport the variable names
  class PXDTrackCombinationVarNames : public
    TrackFindingCDC::VarNames<BaseVXDTrackCombinationFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(pxdTrackCombinationNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return pxdTrackCombinationNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class PXDTrackCombinationVarSet : public TrackFindingCDC::VarSet<PXDTrackCombinationVarNames> {

  public:
    /// Generate and assign the variables from the object.
    bool extract(const BaseVXDTrackCombinationFilter::Object* object) final;
  };
}
