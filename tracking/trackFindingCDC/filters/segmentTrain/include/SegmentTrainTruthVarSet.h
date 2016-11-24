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

#include <vector>
#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class SegmentInformation;

    /// Names of the variables to be generated
    constexpr
    static char const* const segmentTrainTruthVarNames[] = {
      "belongs_to_same_track_truth",
      "truth",
    };

    /// Vehicle class to transport the variable names
    struct SegmentTrainTruthVarNames
      : public VarNames<std::pair<std::vector<SegmentInformation*>, const CDCTrack*>> {

      /// Number of variables to be generated
      static const size_t nVars = size(segmentTrainTruthVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return segmentTrainTruthVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment train to track match
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class SegmentTrainTruthVarSet : public VarSet<SegmentTrainTruthVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>* testPair) final;
    };
  }
}
