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

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     TODO
     */
    class SegmentTrackBestMatchSelector : public SingleMatchSelector<CDCTrack, CDCSegment2D> {
      /// The parent class
      using Super = SingleMatchSelector<CDCTrack, CDCSegment2D>;

    public:
      /// Remove all hits that are in another segment
      void resetNonBestMatches(const typename Super::WeightedRelationItem& notBestMatch) override
      {
        CDCTrack& track = *(notBestMatch.getFrom());
        const CDCSegment2D& segment = *(notBestMatch.getTo());

        // This part is a bit ugly: what we want to do is to remove all hits in the track, which are also part
        // in the segment. Additionally, we unset the taken flag of these hits.
        const auto& hitIsPartOfSegment = [&segment](const CDCRecoHit3D & trackWireHit) {
          bool trackHitIsInSegment = std::find_if(segment.begin(), segment.end(), [&trackWireHit](const CDCRecoHit2D & segmentWireHit) {
            return segmentWireHit.getWireHit() == trackWireHit.getWireHit();
          }) != segment.end();
          if (trackHitIsInSegment) {
            trackWireHit.getWireHit().getAutomatonCell().unsetTakenFlag();
          }

          return trackHitIsInSegment;
        };

        erase_remove_if(track, hitIsPartOfSegment);
      }
    };
  }
}
