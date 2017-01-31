/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/vxdCDCTrackMerging/detectorTrackCombination/BaseDetectorTrackCombinationFilter.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const detectorTrackCombinationWeightVarNames[] = {
    "relation_weight"
  };

  /// Vehicle class to transport the variable names
  class DetectorTrackCombinationWeightVarNames : public TrackFindingCDC::VarNames<BaseDetectorTrackCombinationFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(detectorTrackCombinationWeightVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return detectorTrackCombinationWeightVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class DetectorTrackCombinationWeightVarSet : public TrackFindingCDC::VarSet<DetectorTrackCombinationWeightVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC-pair
    bool extract(const BaseDetectorTrackCombinationFilter::Object* pair) final {
      if (not pair)
      {
        return false;
      }

      var<named("relation_weight")>() = pair->getWeight();
      return true;
    }
  };
}
