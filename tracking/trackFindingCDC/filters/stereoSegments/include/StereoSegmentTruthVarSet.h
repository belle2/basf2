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
    class CDCRecoSegment3D;
    class CDCRecoSegment2D;

    /// Names of the variables to be generated.
    constexpr
    static char const* const stereoSegmentTruthVarNames[] = {
      "truth",
      "track_is_fake_truth",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a stereo hit and a track pair using MC truth.
     */
    class StereoSegmentTruthVarNames : public
      VarNames<std::pair<std::pair<const CDCRecoSegment2D*, const CDCRecoSegment3D>, const CDCTrack&>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(stereoSegmentTruthVarNames);

      /// Get the name of the column.
      constexpr
      static char const* getName(int iName)
      {
        return stereoSegmentTruthVarNames[iName];
      }
    };

    /** Class that computes floating point variables from a stereo segment and a track pair
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class StereoSegmentTruthVarSet : public VarSet<StereoSegmentTruthVarNames> {

    public:
      /// Generate and assign the variables from the pair.
      bool extract(const std::pair<std::pair<const CDCRecoSegment2D*, const CDCRecoSegment3D>,
                   const CDCTrack&>* testPair) override;
    };
  }
}
