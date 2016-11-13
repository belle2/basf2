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
    static char const* const stereoHitTruthNames[] = {
      "truth",
      "truth_may_reversed",
      "track_is_fake_truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a stereo hit and a track pair using MC truth.
     */
    class StereoHitTruthVarNames : public VarNames<std::pair<const CDCRecoHit3D*, const CDCTrack*>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 3;

      /// Get the name of the column.
      constexpr
      static char const* getName(int iName)
      {
        return stereoHitTruthNames[iName];
      }
    };

    /** Class that computes floating point variables from a stereo hit and a track pair
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class StereoHitTruthVarSet : public VarSet<StereoHitTruthVarNames> {

    public:
      /// Construct the peeler.
      explicit StereoHitTruthVarSet() : VarSet<StereoHitTruthVarNames>() { }

      /// Generate and assign the variables from the pair.
      bool extract(const std::pair<const CDCRecoHit3D*, const CDCTrack*>* testPair) override;
    };
  }
}
