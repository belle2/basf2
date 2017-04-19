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
#include <tracking/modules/cdcToVXDExtrapolator/filterBased/BaseCDCTrackSpacePointCombinationFilter.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcTrackSpacePointCombinationVarNames[] = {
    "distance",
    "xy_distance",
    "z_distance",
    "track_position_x",
    "track_position_y",
    "track_position_z",
    "hit_position_x",
    "hit_position_y",
    "hit_position_z",
    "track_position_at_hit_x",
    "track_position_at_hit_y",
    "track_position_at_hit_z",
    "same_hemisphere",
    "layer",
    "ladder",
    "sensor",
    "segment",
    "id",
    "last_layer",
    "last_ladder",
    "last_sensor",
    "last_segment",
    "last_id",
    "pt",
    "tan_lambda",
    "phi",
    "arcLengthOfHitPosition",
    "arcLengthOfCenterPosition",
    "numberOfHoles",
    "chi2"
  };

  /// Vehicle class to transport the variable names
  class CDCTrackSpacePointCombinationVarNames : public TrackFindingCDC::VarNames<BaseCDCTrackSpacePointCombinationFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcTrackSpacePointCombinationVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcTrackSpacePointCombinationVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class CDCTrackSpacePointCombinationVarSet : public TrackFindingCDC::VarSet<CDCTrackSpacePointCombinationVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC-pair
    virtual bool extract(const BaseCDCTrackSpacePointCombinationFilter::Object* pair) override;
  };
}
