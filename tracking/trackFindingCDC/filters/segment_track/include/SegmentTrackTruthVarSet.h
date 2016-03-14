/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;

    /// Names of the variables to be generated.
    constexpr
    static char const* const segmentTrackTruthNames[5] = {
      "belongs_to_same_track_truth",
      "segment_is_fake_truth",
      "segment_purity_truth",
      "track_purity_truth",
      "truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class SegmentTrackTruthVarNames : public VarNames<std::pair<const CDCRecoSegment2D*, const CDCTrack*>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 5;

      constexpr
      static char const* getName(int iName)
      {
        return segmentTrackTruthNames[iName];
      }
    };

    /** Class that computes floating point variables from a wire hit clusters.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class SegmentTrackTruthVarSet : public VarSet<SegmentTrackTruthVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      explicit SegmentTrackTruthVarSet(const std::string& prefix = "") : VarSet<SegmentTrackTruthVarNames>(prefix) { }

      /// Generate and assign the variables from the cluster
      virtual bool extract(const std::pair<const CDCRecoSegment2D*, const CDCTrack*>* testPair) override final;

    };
  }
}
