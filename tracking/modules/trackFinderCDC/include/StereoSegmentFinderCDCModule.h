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
#include <tracking/trackFindingCDC/collectors/stereo_segments/StereoSegmentCollector.h>

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
  }

  namespace TrackFindingCDC {
    class StereoSegmentFinderCDCModule : public TrackFinderCDCFromSegmentsModule {

    public:
      /**
       * Constructor.
       */
      StereoSegmentFinderCDCModule();

      /** Initialize the filter. */
      void initialize() override;

      /** Terminate the filter. */
      void terminate() override;

    private:
      /**
       * Try to combine the segments and the tracks.
       */
      void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

      TrackFindingCDC::StereoSegmentCollectorSingleMatch m_collector;
    };
  }
}
