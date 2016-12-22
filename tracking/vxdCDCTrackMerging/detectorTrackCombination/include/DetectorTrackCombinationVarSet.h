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
  static char const* const detectorTrackCombinationVarNames[] = {
    "phiCDC",
    "phiVXD",
    "thetaCDC",
    "thetaVXD",
    "pCDC",
    "pVXD",
    "phiAbs",
    "phiRel",
    "thetaAbs",
    "thetaRel",
    "pAbs",
    "pRel",
    "chargeVXD",
    "chargeCDC",
    "numberHitsCDC",
    "numberHitsVXD",
    "distance",
    "vertex",
    "vertexCDC",
    "vertexVXD",
    "chargeDif"
  };

  /// Vehicle class to transport the variable names
  class DetectorTrackCombinationVarNames : public TrackFindingCDC::VarNames<BaseDetectorTrackCombinationFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(detectorTrackCombinationVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return detectorTrackCombinationVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match.
   */
  class DetectorTrackCombinationVarSet : public TrackFindingCDC::VarSet<DetectorTrackCombinationVarNames> {

  public:
    /// Generate and assign the variables from the VXD-CDC-pair
    virtual bool extract(const BaseDetectorTrackCombinationFilter::Object* pair) override;
  };
}
