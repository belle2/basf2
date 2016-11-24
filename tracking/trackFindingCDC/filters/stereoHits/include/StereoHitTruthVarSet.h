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

    /// Names of the variables to be generated
    constexpr
    static char const* const stereoHitTruthVarNames[] = {
      "truth",
      "truth_may_reversed",
      "track_is_fake_truth",
    };

    /// Vehicle class to transport the variable names
    struct StereoHitTruthVarNames : public VarNames<std::pair<const CDCRecoHit3D*, const CDCTrack*>> {

      /// Number of variables to be generated
      static const size_t nVars = size(stereoHitTruthVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return stereoHitTruthVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a stereo hit to track match
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class StereoHitTruthVarSet : public VarSet<StereoHitTruthVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const std::pair<const CDCRecoHit3D*, const CDCTrack*>* testPair) override;
    };
  }
}
