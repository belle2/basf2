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

#include <tracking/ckf/svd/entities/CKFToSVDResult.h>
#include <tracking/ckf/svd/utilities/SVDAdvancer.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const relationSVDResultVarNames[] = {
    "pt",
    "theta",

    "cdc_lowest_layer",
    "svd_highest_layer",

    "number_of_hits",
    "number_of_holes",

    "last_hit_layer",
    "first_hit_layer",

    "distance_to_cdc_track",
    "distance_to_cdc_track_xy",

    "number_of_hits_related_svd_track",

    "chi2",
    "chi2_vxd_max",
    "chi2_vxd_min",
    "chi2_cdc"
  };

  /// Vehicle class to transport the variable names
  class RelationSVDResultVarNames : public TrackFindingCDC::VarNames<CKFToSVDResult> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(relationSVDResultVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return relationSVDResultVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class RelationSVDResultVarSet : public TrackFindingCDC::VarSet<RelationSVDResultVarNames> {
    using Super = TrackFindingCDC::VarSet<RelationSVDResultVarNames>;

  public:
    RelationSVDResultVarSet();

    /// Generate and assign the variables from the object.
    bool extract(const CKFToSVDResult* object) final;

    /// Set the direction of the advancer
    void initialize() override;
  private:
    /// Findlet for advancing
    SVDAdvancer m_advancer;
  };
}
