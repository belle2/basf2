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
#include <tracking/trackFindingCDC/collectors/base/QuadTreeBasedMatcher.h>
#include <tracking/trackFindingCDC/hough/z0_tanLambda/SegmentZ0TanLambdaLegendre.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCSegment2D;

    /**
     * A matcher algorithm for using a stereo quad tree for matching stereo segments
     * to tracks. After the quad tree has given a yes to a match, the configured filter is used to
     * give a weight to the relation.
     */
    class StereoSegmentTrackMatcherQuadTree : public QuadTreeBasedMatcher<SegmentZ0TanLambdaLegendre> {
    public:
      /// Use tracks as collector items.
      using CollectorItem = CDCTrack;

      /// Use (stereo) segments as collection items.
      using CollectionItem = CDCSegment2D;

      /// Expose the parameters to the module.
      void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix) override;

      /**
       * Use the given filter (via the module parameters) to find a matching.
       */
      std::vector<WithWeight<const CollectionItem*> >
      match(const CollectorItem& track, const std::vector<CollectionItem>& segments);

    private:
      /// Parameters
      /// Set to false to skip the B2B check (good for curlers).
      bool m_param_checkForB2BTracks = true;

      /**
       * Before filling the quad tree, check each reconstructed segment to be applicable to this track.
       * @param segment3D the reconstructed segment that should be matched to the trac
       * @param isCurler a flag if the track is a curler
       * @param shiftValue 2 * TMath::Pi() * radius of the track
       * @param lastSuperLayer the last superlayer of the track
       * @param lastArcLength2D the last arc length 2d of the track
       * @return True, if the match can be m ade (the quad tree has still to give his yes too).
       */
      bool checkSegment3D(CDCSegment3D& segment3D,
                          bool isCurler,
                          double shiftValue,
                          ISuperLayer lastSuperLayer,
                          double lastArcLength2D) const;
    };
  }
}
