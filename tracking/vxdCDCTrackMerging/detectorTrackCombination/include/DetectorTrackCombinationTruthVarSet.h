/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Malwin Weiler, Nils Braun                                *
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
  static char const* const detectorTrackCombinationTruthNames[] = {
    "truth"
  };

  /// Vehicle class to transport the variable names
  class DetectorTrackCombinationTruthVarNames : public TrackFindingCDC::VarNames<BaseDetectorTrackCombinationFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(detectorTrackCombinationTruthNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return detectorTrackCombinationTruthNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class DetectorTrackCombinationTruthVarSet : public TrackFindingCDC::VarSet<DetectorTrackCombinationTruthVarNames> {

  public:
    /// Generate and assign the variables from the pair.
    virtual bool extract(const BaseDetectorTrackCombinationFilter::Object* pair) override;
  };
}
