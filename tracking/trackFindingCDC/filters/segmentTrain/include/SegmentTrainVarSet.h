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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class SegmentInformation;

    /// Names of the variables to be generated
    constexpr
    static char const* const segmentTrainVarNames[] = {
      "is_stereo",
      "maximum_perpS_overlap",
      "size",
      "perpS_overlap_mean",
      "calculation_failed",
    };

    /// Vehicle class to transport the variable names
    struct SegmentTrainVarNames : public VarNames<std::pair<std::vector<SegmentInformation*>, const CDCTrack*>> {

      /// Number of variables to be generated
      static const size_t nVars = size(segmentTrainVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return segmentTrainVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment train to track match
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class SegmentTrainVarSet : public VarSet<SegmentTrainVarNames> {

    public:
      /// We use this amount of overlap when defining a segment train
      static constexpr const float m_param_percentageForPerpSMeasurements = 0.05;

      /// Generate and assign the contained variables
      bool extract(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>* testPair) final;
    };
  }
}
