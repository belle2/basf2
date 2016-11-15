/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/collectors/base/FilterBasedMatcher.h>

#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentFilterFactory.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoSegment2D;

    /// A Matcher Algorithm for adding stereo segments to tracks using a configurable filter (e.g. TMVA).
    class StereoSegmentTrackMatcherFilter : public FilterBasedMatcher<StereoSegmentFilterFactory> {
    public:
      /// Use tracks as collector items.
      using CollectorItem = CDCTrack;

      /// Use segments as collection items.
      using CollectionItem = CDCRecoSegment2D;

      /// Use the given filter (via the module parameters) to find a matching.
      std::vector<WithWeight<const CollectionItem*> >
      match(const CDCTrack& track, const std::vector<CDCRecoSegment2D>& recoSegments);
    };
  }
}
