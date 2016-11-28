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
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCTrack;

    /// Filter for the construction of good segment - track pairs
    class SimpleSegmentTrackFilter : public FilterOnVarSet<SegmentTrackVarSet> {

    private:
      /// Type of the super class
      using Super = FilterOnVarSet<SegmentTrackVarSet>;

    public:
      /// Get the output of the filter.
      Weight operator()(const std::pair<const CDCSegment2D*, const CDCTrack*>& testPair) final;
    };
  }
}
