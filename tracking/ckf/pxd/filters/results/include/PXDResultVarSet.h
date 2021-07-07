/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/varsets/FixedSizeNamedFloatTuple.h>

#include <tracking/ckf/pxd/entities/CKFToPXDResult.h>
#include <tracking/ckf/pxd/utilities/PXDAdvancer.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const pxdResultVarNames[] = {
    "chi2_vxd_max",
    "chi2_vxd_min",
    "chi2_seed",
    "chi2",

    "number_of_hits",

    "pt",
    "theta",

    "number_of_holes",

    "last_hit_layer",
    "first_hit_layer",

    "weight_sum",

    "has_missing_layer_1",
    "has_missing_layer_2",
    "has_missing_layer_3",
    "has_missing_layer_4",
    "has_missing_layer_5",
    "has_missing_layer_6",

    "number_of_overlap_hits",

    "distance_to_seed_track",
    "distance_to_seed_track_xy",
  };

  /// Vehicle class to transport the variable names
  class PXDResultVarNames : public TrackFindingCDC::VarNames<CKFToPXDResult> {

  public:
    /// Number of variables to be generated.
    // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
    // at least tell cppcheck that everything is fine
    // cppcheck-suppress duplInheritedMember
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
    /// Type of the base class
    using Super = TrackFindingCDC::VarSet<PXDResultVarNames>;

  public:
    PXDResultVarSet();

    void initialize() override;

    /// Generate and assign the variables from the object.
    bool extract(const CKFToPXDResult* object) final;

  private:
    /// Findlet for advancing
    PXDAdvancer m_advancer;
  };
}
