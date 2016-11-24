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
    class CDCSegment3D;

    /// Names of the variables to be generated
    constexpr
    static char const* const stereoSegmentTruthVarNames[] = {
      "truth",
      "track_is_fake_truth",
    };

    /// Vehicle class to transport the variable names
    struct StereoSegmentTruthVarNames : public VarNames<std::pair<const CDCSegment3D*, const CDCTrack*>> {

      /// Number of variables to be generated
      static const size_t nVars = size(stereoSegmentTruthVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return stereoSegmentTruthVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a stereo segment to track match
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class StereoSegmentTruthVarSet : public VarSet<StereoSegmentTruthVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const std::pair<const CDCSegment3D*, const CDCTrack*>* testPair) override;
    };
  }
}
