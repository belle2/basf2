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

#include <tracking/ckf/svd/entities/CKFToSVDResult.h>
#include <tracking/ckf/svd/utilities/SVDAdvancer.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const svdResultVarNames[] = {
    "chi2_vxd_max",
    "chi2_vxd_min",
    "chi2_cdc",
    "chi2",

    "number_of_hits",

    "pt",
    "theta",

    "number_of_holes",

    "cdc_lowest_layer",

    "last_hit_layer",
    "first_hit_layer",

    "weight_sum",

    "has_missing_layer_1",
    "has_missing_layer_2",
    "has_missing_layer_3",
    "has_missing_layer_4",
    "has_missing_layer_5",
    "has_missing_layer_6",

    "distance_to_cdc_track",
    "distance_to_cdc_track_xy",
  };

  /// Vehicle class to transport the variable names
  class SVDResultVarNames : public TrackFindingCDC::VarNames<CKFToSVDResult> {

  public:
    /// Number of variables to be generated.
    // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
    // at least tell cppcheck that everything is fine
    // cppcheck-suppress duplInheritedMember
    static const size_t nVars = TrackFindingCDC::size(svdResultVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return svdResultVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class SVDResultVarSet : public TrackFindingCDC::VarSet<SVDResultVarNames> {
    /// Type of the base class
    using Super = TrackFindingCDC::VarSet<SVDResultVarNames>;

  public:
    SVDResultVarSet();

    /// Generate and assign the variables from the object.
    bool extract(const CKFToSVDResult* object) final;

    void initialize() override;

  private:
    /// Findlet for advancing
    SVDAdvancer m_advancer;
  };
}
