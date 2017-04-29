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

#include <tracking/ckf/filters/cdcTrackSpacePointCombination/BaseCDCTrackSpacePointCombinationFilter.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcTrackSpacePointCombinationTruthNames[] = {
    "truth",
    "truth_no_curler",
    "truth_position_x",
    "truth_position_y",
    "truth_position_z",
    "truth_momentum_x",
    "truth_momentum_y",
    "truth_momentum_z",
    "event_id",
    "space_point_number",
    "cdc_number"
  };

  /// Vehicle class to transport the variable names
  class CDCTrackSpacePointCombinationTruthVarNames : public
    TrackFindingCDC::VarNames<BaseCDCTrackSpacePointCombinationFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcTrackSpacePointCombinationTruthNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcTrackSpacePointCombinationTruthNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class CDCTrackSpacePointCombinationTruthVarSet : public TrackFindingCDC::VarSet<CDCTrackSpacePointCombinationTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCDCTrackSpacePointCombinationFilter::Object* object) override;
  };
}
