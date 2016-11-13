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
#include <string>
#include <cassert>


namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class SegmentInformation;

    /// Names of the variables to be generated.
    constexpr
    static char const* const segmentTrainNames[] = {
      "is_stereo",
      "maximum_perpS_overlap",
      "size",
      "perpS_overlap_mean",
      "calculation_failed"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class SegmentTrainVarNames : public
      VarNames<std::pair<std::vector<SegmentInformation*>, const CDCTrack*>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(segmentTrainNames);

      /// Get the name of the corresponding column.
      constexpr
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
      /// We use this amount of overlap when defining a segment train
      static constexpr const float m_param_percentageForPerpSMeasurements = 0.05;

      /// Construct the peeler.
      explicit SegmentTrainVarSet() : VarSet<SegmentTrainVarNames>() { }

      /// Generate and assign the variables from the pair
      bool
      extract(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>* testPair) final;
    };
  }
}
