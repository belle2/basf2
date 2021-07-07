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

#include <tracking/ckf/vtx/entities/CKFToVTXResult.h>
#include <tracking/ckf/vtx/utilities/VTXAdvancer.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const vtxResultVarNames[] = {
    "chi2_vtx_max",
    "chi2_vtx_min",
    "chi2_cdc",
    "chi2",

    "number_of_hits",
    "number_of_used_layers",

    "pt",
    "theta",

    "cdc_lowest_layer",

    "last_hit_layer",
    "first_hit_layer",

    "weight_sum",

    // ATTENTION: Here we have a hardcoded number of layers. Change if there is a different number of layers in the VTX
    "has_missing_layer_1",
    "has_missing_layer_2",
    "has_missing_layer_3",
    "has_missing_layer_4",
    "has_missing_layer_5",
    "has_missing_layer_6",
    "has_missing_layer_7",

    "distance_to_cdc_track",
    "distance_to_cdc_track_xy",
  };

  /// Vehicle class to transport the variable names
  class VTXResultVarNames : public TrackFindingCDC::VarNames<CKFToVTXResult> {

  public:
    /// Number of variables to be generated.
    // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
    // at least tell cppcheck that everything is fine
    // cppcheck-suppress duplInheritedMember
    static const size_t nVars = TrackFindingCDC::size(vtxResultVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return vtxResultVarNames[iName];
    }
  };

  /**
   * Var set used in the VTX-CDC-Merger for calculating the probability of a VTX-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class VTXResultVarSet : public TrackFindingCDC::VarSet<VTXResultVarNames> {
    /// Type of the base class
    using Super = TrackFindingCDC::VarSet<VTXResultVarNames>;

  public:
    VTXResultVarSet();

    void initialize() override;

    /// Generate and assign the variables from the object.
    bool extract(const CKFToVTXResult* object) final;

  private:
    /// Findlet for advancing
    VTXAdvancer m_advancer;
  };
}
