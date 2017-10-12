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
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/varsets/FixedSizeNamedFloatTuple.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/BaseCKFCDCToSpacePointStateFilter.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const ckfCDCToSpacePointStateBasicVarNames[] = {
    "distance",
    "xy_distance",
    "z_distance",

    "mSoP_distance",
    "mSoP_xy_distance",
    "mSoP_z_distance",

    "same_hemisphere",

    "layer",
    "number",
    "overlap",

    "arcLengthOfHitPosition",
    "arcLengthOfCenterPosition",

    "numberOfHoles",

    "pt",
    "tan_lambda",
    "phi",

    "sigma_uv",
    "residual_over_sigma",

    "residual",

    "chi2",

    "ladder",
    "sensor",
    "segment",
    "id",

    "last_layer",
    "last_ladder",
    "last_sensor",
    "last_segment",
    "last_id",
  };

  /// Vehicle class to transport the variable names
  class CKFCDCToSpacePointStateBasicVarNames : public
    TrackFindingCDC::VarNames<BaseCKFCDCToSpacePointStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(ckfCDCToSpacePointStateBasicVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return ckfCDCToSpacePointStateBasicVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class CKFCDCToSpacePointStateBasicVarSet : public TrackFindingCDC::VarSet<CKFCDCToSpacePointStateBasicVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC-pair
    virtual bool extract(const BaseCKFCDCToSpacePointStateFilter::Object* pair) override;
  };
}
