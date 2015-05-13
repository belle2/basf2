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

#include <tracking/trackFindingCDC/varsets/EmptyVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>


namespace Belle2 {
  namespace TrackFinderOutputCombining {
    class SegmentInformation;
  }
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const segmentTrainNames[5] = {
      "is_stereo",
      "maximum_perpS_overlap",
      "size",
      "perpS_overlap_mean"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class SegmentTrainVarNames : public
      VarNames<std::pair<std::vector<TrackFinderOutputCombining::SegmentInformation*>, const CDCTrack*>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 4;

      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return segmentTrainNames[iName];
      }
    };

    /** Class that computes floating point variables from a pair of track and segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class SegmentTrainVarSet : public VarSet<SegmentTrainVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      SegmentTrainVarSet(const std::string& prefix = "") : VarSet<SegmentTrainVarNames>(prefix) { }

      /// Generate and assign the variables from the pair
      virtual bool extract(const std::pair<std::vector<TrackFinderOutputCombining::SegmentInformation*>, const CDCTrack*>* testPair)
      IF_NOT_CINT(override final);
    };
  }
}
