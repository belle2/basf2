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

#include <tracking/trackFindingCDC/collectors/selectors/SingleMatchSelector.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     Selector based on a SingleMatchSelector, for extracting those combinations of tracks and segments,
     which should later be merged.
     It inherits the functionality of the SingleMatchSelector, meaning it will only output those
     combinations, which are best for all segments and tracks.
     Additionaly, it will mark all shared hits between segments and tracks, which are *not* part of the best
     combinations as masked. This means, if the algorithm determines that a segment should belong to a certain track,
     all hits that are part of other tracks can later be removed from those tracks. This will increase the hit purity.
     */
    class SegmentTrackBestMatchSelector : public SingleMatchSelector<CDCTrack, CDCSegment2D> {
      /// The parent class
      using Super = SingleMatchSelector<CDCTrack, CDCSegment2D>;

    public:
      /**
       * Mark all hits that are in another, non-optimal segment as masked. This information will later be used to
       * remove those hits from the tracks.
       */
      void resetNonBestMatches(const typename Super::WeightedRelationItem& notBestMatch) override
      {
        CDCTrack& track = *(notBestMatch.getFrom());
        const CDCSegment2D& segment = *(notBestMatch.getTo());

        for (const CDCRecoHit3D& trackWireHit : track) {
          const auto& hitIsInSegment = [&trackWireHit](const CDCRecoHit2D & segmentWireHit) {
            return segmentWireHit.getWireHit() == trackWireHit.getWireHit();
          };
          const bool trackHitIsInSegment = any(segment, hitIsInSegment);
          if (trackHitIsInSegment) {
            trackWireHit.getWireHit().getAutomatonCell().setMaskedFlag();
          }
        };
      }
    };
  }
}
