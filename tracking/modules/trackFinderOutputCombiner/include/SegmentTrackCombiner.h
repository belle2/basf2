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
#include <tracking/trackFinderOutputCombining/Lookups.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
  }

  class SegmentTrackCombinerModule : public TrackFinderCDCFromSegmentsModule {

  public:
    /**
     * Constructor.
     */
    SegmentTrackCombinerModule(): TrackFinderCDCFromSegmentsModule(), m_combiner() { }

    /**
     * Used the QuadTree to generate tracks from segments.
     */
    void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

  private:
    TrackFinderOutputCombining::SegmentTrackCombiner m_combiner;
  };
}
