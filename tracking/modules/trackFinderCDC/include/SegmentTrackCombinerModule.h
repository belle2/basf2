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
#include <tracking/trackFindingCDC/trackFinderOutputCombining/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackRejecter.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCTrack;
  }

  namespace TrackFindingCDC {
    /**
     * Module for the combination of tracks and segments.
     *
     * This module uses configurable filters to create matches between segments (from the local finder)
     * and tracks (from the global finder). It also has capabilities to filter the remaining tracks for fakes
     * and the remaining segments for new tracks/background.
     *
     * Use this module at the end of the path, after you have run the local and the global track finder.
     *
     * It uses several (filtering) steps, to do its job, namely:
     *  1. Creation of a fast segment and track lookup.
     *  2. First matching of segment and tracks that share one or more hits. \label{list-start}
     *  3. Deletion of fake segments. \label{list-fakes}
     *  4. Flagging of segments belonging to particles that probably can not be found by the global track finder (and can therefore not be matched).
     *  5. Matching of the remaining segments with the tracks or among themselves and then with the tracks (not used in the following). \label{list-second}
     *  6. Filtering of fake tracks in the made combinations.  \label{list-end}
     *  7. Cleanup of the lookup cache.
     */
    class SegmentTrackCombinerModule : public TrackFinderCDCFromSegmentsModule {

    private:
      /// Type of the base class
      using Super = TrackFinderCDCFromSegmentsModule;

    public:
      /// Constructor setting up the filter parameters
      SegmentTrackCombinerModule();

      /// Initialize the filters
      void initialize() override;

      /// Signal new run to the filters
      void beginRun() override;

      /// Main event handler
      void event() override;

      /// Signal end of run to the filters
      void endRun() override;

      /// Terminate the filters
      void terminate() override;

    private:
      /// Try to combine the segments and the tracks.
      void generate(std::vector<TrackFindingCDC::CDCSegment2D>& segments,
                    std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

    private:
      /// Object that handles the combination.
      SegmentTrackCombiner m_combiner;

      // Filters
      /// Reference to the chooser to be used for matching segments and tracks in the first step.
      Chooseable<BaseSegmentTrackFilter> m_chooseableSegmentTrackFilterFirstStep;

      /// Reference to the segment filter.
      Chooseable<BaseSegmentFilter> m_chooseableSegmentFilter;

      /// Reference to the new segment filter.
      Chooseable<BaseNewSegmentFilter> m_chooseableNewSegmentFilter;

      /// Reference to the chooser to be used for matching segments and tracks in the second step.
      Chooseable<BaseSegmentTrackFilter> m_chooseableSegmentTrackFilterSecondStep;

      /// Reference to the filter to be used for construction segment trains.
      Chooseable<BaseSegmentTrainFilter> m_chooseableSegmentTrainFilter;

      /// Reference to the filter to be used to do an unique segment train <-> track matching.
      Chooseable<BaseSegmentInformationListTrackFilter> m_chooseableSegmentInformationListTrackFilter;

      /// Findlet to filter out fake tracks
      TrackRejecter m_trackRejecter;
    };
  }
}
