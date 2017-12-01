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

#include <tracking/ckf/pxd/entities/CKFToPXDResult.h>
#include <tracking/ckf/pxd/utilities/PXDAdvancer.h>
#include <tracking/ckf/pxd/utilities/PXDKalmanStepper.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const pxdResultVarNames[] = {
    "chi2_vxd_full",
    "chi2_vxd_max",
    "chi2_vxd_min",
    "chi2_vxd_mean",
    "number_of_hits",
    "pt",
    "chi2_seed",
    "number_of_holes",
    "last_hit_layer",
    "first_hit_layer",
    "chi2",
    "prob",
    "has_missing_layer_1",
    "has_missing_layer_2",
    "has_missing_layer_3",
    "has_missing_layer_4",
    "has_missing_layer_5",
    "has_missing_layer_6",
    "number_of_overlap_hits",
    "theta",
    "distance_to_seed_track",
    "distance_to_seed_track_xy",
  };

  /// Vehicle class to transport the variable names
  class PXDResultVarNames : public TrackFindingCDC::VarNames<CKFToPXDResult> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(pxdResultVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return pxdResultVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class PXDResultVarSet : public TrackFindingCDC::VarSet<PXDResultVarNames> {

  public:
    void initialize() override;

    /// Generate and assign the variables from the object.
    bool extract(const CKFToPXDResult* object) final;

  private:
    /// Findlet for advancing
    PXDAdvancer m_advancer;
    /// Findlet for kalman step
    PXDKalmanStepper m_kalmanStepper;
  };
}
