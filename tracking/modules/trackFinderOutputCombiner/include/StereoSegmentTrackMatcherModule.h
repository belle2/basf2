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

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>
#include <set>

namespace Belle2 {

  /// Matches the found stereo segments from the local track finder to the found tracks from the legendre track finder after stereo assignment.
  /// It uses hit based information - that means it matches a segment and a track if they both share hits and if the matching combination is unique.
  class StereoSegmentTrackMatcherModule : public TrackFinderCDCFromSegmentsModule {

  public:

    /**
     * Constructor to set the module parameters.
     */
    StereoSegmentTrackMatcherModule();

  private:
    /**
     * Try to match the stereo segments and the tracks
     */
    void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

    /**
     * Compile a hit->segment lookup for later speed up
     */
    void fillHitLookUp(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments);

    /**
     * Compile a segment->track lookup.
     */
    void fillTrackLookUp(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks);

    /**
     * Find the matching segment to a reco hit
     */
    const TrackFindingCDC::CDCRecoSegment2D* findMatchingSegment(const TrackFindingCDC::CDCRecoHit3D& recoHit);

    /// Object pools

    /// A map from the found CDCHits to the RecoSegments. Uses pointers to the elements given to generate ( = elements in the StoreVector)
    std::map<const CDCHit*, const TrackFindingCDC::CDCRecoSegment2D*> m_hitSegmentLookUp;

    /// A map from the found Segments to the CDCTracks. Uses pointers to the elements given to generate ( = elements in the StoreVector)
    std::map<const TrackFindingCDC::CDCRecoSegment2D*, std::set<TrackFindingCDC::CDCTrack*>> m_segmentTrackLookUp;
  };
}
