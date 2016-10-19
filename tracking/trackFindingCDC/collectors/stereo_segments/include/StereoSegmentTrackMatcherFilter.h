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
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <tracking/trackFindingCDC/collectors/base/FilterBasedMatcher.h>
#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentFilterFactory.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/core/ModuleParamList.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoSegment2D;

    /// A Matcher Algorithm for adding stereo segments to tracks using a configurable filter (e.g. TMVA).
    class StereoSegmentTrackMatcherFilter : public FilterBasedMatcher<StereoSegmentFilterFactory> {
    public:
      /// Use tracks as collector items.
      typedef CDCTrack CollectorItem;
      /// Use segments as collection items.
      typedef CDCRecoSegment2D CollectionItem;

      /// Empty desctructor. Everything is handled via terminate.
      virtual ~StereoSegmentTrackMatcherFilter() = default;

      /**
       * Use the given filter (via the module parameters) to find a matching.
       */
      std::vector<WithWeight<const CollectionItem*>> match(const CollectorItem& collectorItem,
                                                           const std::vector<CollectionItem>& collectionList);
    };
  }
}
