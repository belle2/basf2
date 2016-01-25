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
      "track_is_fake_truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment.
     */
    class StereoHitTruthVarNames : public VarNames<std::pair<const CDCRecoHit3D*, const CDCTrack*>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 2;

      constexpr
      static char const* getName(int iName)
      {
        return stereoHitTruthNames[iName];
      }
    };

    /** Class that computes floating point variables from a segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class StereoHitTruthVarSet : public VarSet<StereoHitTruthVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      explicit StereoHitTruthVarSet(const std::string& prefix = "") : VarSet<StereoHitTruthVarNames>(prefix) { }

      /// Generate and assign the variables from the cluster
      virtual bool extract(const std::pair<const CDCRecoHit3D*, const CDCTrack*>* testPair) override;

    };
  }
}
