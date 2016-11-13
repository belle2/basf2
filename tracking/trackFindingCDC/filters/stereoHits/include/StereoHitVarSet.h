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


namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoHit3D;

    /// Names of the variables to be generated.
    constexpr
    static char const* const stereoHitVarNames[] = {
      "track_size",
      "pt",
      "reco_s",
      "reco_z",
      "phi_track",
      "phi_hit",
      "theta_hit",
      "drift_length",
      "adc_count",
      "xy_distance_zero_z",
      "right_hit",
      "track_back_s",
      "track_front_s",
      "track_mean_s",
      "s_distance",
      "track_radius",
      "superlayer_id"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a stereo hit and a track pair.
     */
    class StereoHitVarNames : public VarNames<std::pair<const CDCRecoHit3D*, const CDCTrack*>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(stereoHitVarNames);

      /// Get the name of the column.
      constexpr
      static char const* getName(int iName)
      {
        return stereoHitVarNames[iName];
      }
    };

    /** Class that computes floating point variables from a stereo hit and a track pair
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class StereoHitVarSet : public VarSet<StereoHitVarNames> {

    public:
      /// Generate and assign the variables from the pair
      bool extract(const std::pair<const CDCRecoHit3D*, const CDCTrack*>* testPair) override;
    };
  }
}
