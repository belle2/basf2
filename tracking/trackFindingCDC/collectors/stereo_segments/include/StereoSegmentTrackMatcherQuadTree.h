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

#include <tracking/trackFindingCDC/hough/z0_tanLambda/SegmentZ0TanLambdaLegendre.h>

#include <tracking/trackFindingCDC/collectors/base/QuadTreeBasedMatcher.h>
#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentFilterFactory.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/core/ModuleParamList.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoSegment2D;

    class StereoSegmentTrackMatcherQuadTree : public QuadTreeBasedMatcher<SegmentZ0TanLambdaLegendre> {
    public:
      typedef CDCTrack CollectorItem;
      typedef CDCRecoSegment2D CollectionItem;

      /// Empty desctructor. Everything is handled via terminate.
      virtual ~StereoSegmentTrackMatcherQuadTree() { }

      /// Expose the parameters to the module.
      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "");

      /**
       * Use the given filter (via the module parameters) to find a matching.
       */
      std::vector<WithWeight<const CollectionItem*>> match(const CollectorItem& collectorItem,
                                                           const std::vector<CollectionItem>& collectionList);

    private:
      /// Parameters
      /// Set to false to skip the B2B check (good for curlers).
      bool m_param_checkForB2BTracks = true;
      /// Set to true to output debug information.
      bool m_param_writeDebugInformation = false;

      bool checkRecoSegment3D(CDCRecoSegment3D& recoSegment3D, const bool isCurler, const double shiftValue) const;
    };
  }
}
