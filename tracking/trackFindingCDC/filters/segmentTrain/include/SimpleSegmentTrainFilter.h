/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainVarSet.h>

#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the construction of good segment trains
    class SimpleSegmentTrainFilter : public FilterOnVarSet<SegmentTrainVarSet> {

    private:
      /// Type of the super class
      using Super = FilterOnVarSet<SegmentTrainVarSet>;

    public:
      /// Get the output of the filter.
      Weight
      operator()(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>& testPair) final;
    };
  }
}
