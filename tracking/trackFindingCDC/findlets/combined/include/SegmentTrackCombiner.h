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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackRejecter.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackNormalizer.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackFilterFactory.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    /**
     * Findlet for the combination of tracks and segments.
     *
     * This module uses configurable filters to create matches between segments (from the local finder)
     * and tracks (from the global finder). It also has capabilities to filter the remaining tracks for fakes
     * and the remaining segments for new tracks/background.
     *
     * Use this module at the end of the path, after you have run the local and the global track finder.
     *
     * It uses several (filtering) steps, to do its job, namely:
     *  1. Creation of a fast segment and track lookup.
     *  2. First matching of segment and tracks that share one or more hits.
     *  3. Filtering of fake tracks in the made combinations.
     *  4. Cleanup of the lookup cache.
     */
    class SegmentTrackCombinerFindlet : public Findlet<CDCSegment2D&, CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCSegment2D&, CDCTrack&>;

    public:
      /// Constructor setting up the filter parameters
      SegmentTrackCombinerFindlet();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Signal the beginning of a new event
      //void beginEvent() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Try to combine the segments and the tracks.
      void apply(std::vector<CDCSegment2D>& segments,
                 std::vector<CDCTrack>& tracks) override;

    private:
      // Findlets
      // TODO: remove this base class in favour of collector framework
      /// Object that handles the combination.
      SegmentTrackCombiner m_combiner;

      /// Reference to the chooser to be used for matching segments and tracks in the first step.
      ChooseableFilter<SegmentTrackFilterFactory> m_chooseableSegmentTrackFilter;

      /// Findlet to filter out fake tracks
      TrackRejecter m_trackRejecter;

      /// Findlet for normalizing the tracks
      TrackNormalizer m_trackNormalizer;
    };
  }
}
