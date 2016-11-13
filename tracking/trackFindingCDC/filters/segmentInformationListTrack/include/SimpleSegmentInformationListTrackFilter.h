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

#include <tracking/trackFindingCDC/filters/segmentInformationListTrack/SegmentInformationListTrackFilter.h>

#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the construction of good segment trains - track pairs
    class SimpleSegmentInformationListTrackFilter : public BaseSegmentInformationListTrackFilter {

    public:
      /// Constructor
      SimpleSegmentInformationListTrackFilter() : BaseSegmentInformationListTrackFilter() { }

    public:
      /// Get the filter output.
      Weight
      operator()(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>& testPair) final;
    };
  }
}
