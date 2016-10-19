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

    /**
     * A matcher algorithm for using a stereo quad tree for matching stereo segments
     * to tracks. After the quad tree has given a yes to a match, the configured filter is used to
     * give a weight to the relation.
     */
    class StereoSegmentTrackMatcherQuadTree : public QuadTreeBasedMatcher<SegmentZ0TanLambdaLegendre> {
    public:
      /// Use tracks as collector items.
      typedef CDCTrack CollectorItem;
      /// Use (stereo) segments as collection items.
      typedef CDCRecoSegment2D CollectionItem;

      /// Empty desctructor. Everything is handled via terminate.
      virtual ~StereoSegmentTrackMatcherQuadTree() = default;

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

      /**
       * Before filling the quad tree, check each reconstructed segment to be applicable to this track.
       * @param recoSegment3D the reconstructed segment that should be matched to the trac
       * @param isCurler a flag if the track is a curler
       * @param shiftValue 2 * TMath::Pi() * radius of the track
       * @param lastSuperLayer the last superlayer of the track
       * @param lastArcLength2D the last arc length 2d of the track
       * @return True, if the match can be m ade (the quad tree has still to give his yes too).
       */
      bool checkRecoSegment3D(CDCRecoSegment3D& recoSegment3D, const bool isCurler, const double shiftValue,
                              const ISuperLayer lastSuperLayer, const double lastArcLength2D) const;
    };
  }
}
