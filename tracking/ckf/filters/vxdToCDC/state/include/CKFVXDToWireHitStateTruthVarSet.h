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

#include <tracking/ckf/filters/vxdToCDC/state/BaseCKFCKFVXDToWireHitStateFilter.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const ckfVXDToWireHitStateTruthVarNames[] = {
    "truth",
    "truth_position_x",
    "truth_position_y",
    "truth_position_z",
    "truth_momentum_x",
    "truth_momentum_y",
    "truth_momentum_z",
    "truth_event_id",
    "truth_cdc_hit_number",
    "truth_vxd_number"
  };

  /// Vehicle class to transport the variable names
  class CKFVXDToWireHitStateTruthVarNames : public
    TrackFindingCDC::VarNames<BaseCKFCKFVXDToWireHitStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(ckfVXDToWireHitStateTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return ckfVXDToWireHitStateTruthVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class CKFVXDToWireHitStateTruthVarSet : public TrackFindingCDC::VarSet<CKFVXDToWireHitStateTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCKFCKFVXDToWireHitStateFilter::Object* object) override;
  };
}
