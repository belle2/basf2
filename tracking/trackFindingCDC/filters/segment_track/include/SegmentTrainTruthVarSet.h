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

#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrainVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const segmentTrainTruthNames[2] = {
      "belongs_to_same_track_truth",
      "truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class SegmentTrainTruthVarNames : public
      VarNames<std::pair<std::vector<SegmentInformation*>, const CDCTrack*>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 2;

      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return segmentTrainTruthNames[iName];
      }

      /// Marking that the basic cluster variables should be included.
      typedef SegmentTrainVarSet NestedVarSet;
    };

    /** Class that computes floating point variables from a wire hit clusters.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class SegmentTrainTruthVarSet : public VarSet<SegmentTrainTruthVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      SegmentTrainTruthVarSet(const std::string& prefix = "") : VarSet<SegmentTrainTruthVarNames>(prefix) { }

      /// Generate and assign the variables from the cluster
      virtual bool extract(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>* testPair)
      IF_NOT_CINT(override final);

    };
  }
}
