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

#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>

#include <string>
#include <utility>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class SegmentInformation;

    /// Names of the variables to be generated.
    constexpr
    static char const* const segmentTrainTruthNames[] = {
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
      static const size_t nNames = size(segmentTrainTruthNames);

      /// Get the name of the corresponding column.
      constexpr
      static char const* getName(int iName)
      {
        return segmentTrainTruthNames[iName];
      }
    };

    /** Class that computes floating point variables from a wire hit clusters.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class SegmentTrainTruthVarSet : public VarSet<SegmentTrainTruthVarNames> {

    public:
      /// Construct the peeler.
      explicit SegmentTrainTruthVarSet() : VarSet<SegmentTrainTruthVarNames>() { }

      /// Generate and assign the variables from the cluster
      bool
      extract(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>* testPair) final;
    };
  }
}
