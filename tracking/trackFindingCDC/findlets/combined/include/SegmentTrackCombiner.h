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

#include <tracking/trackFindingCDC/collectors/matchers/SharingHitsMatcher.h>
#include <tracking/trackFindingCDC/collectors/selectors/CutSelector.h>
#include <tracking/trackFindingCDC/collectors/selectors/FilterSelector.h>
#include <tracking/trackFindingCDC/collectors/selectors/SingleMatchSelector.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackNormalizer.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentTrackAdderWithNormalization.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackRejecter.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackFilterFactory.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCSegment2D;

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
    class SegmentTrackCombiner : public Findlet<CDCSegment2D&, CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCSegment2D&, CDCTrack&>;

    public:
      /// Constructor setting up the filter parameters
      SegmentTrackCombiner();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Signal the beginning of a new event
      void beginEvent() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Try to combine the segments and the tracks.
      void apply(std::vector<CDCSegment2D>& segments,
                 std::vector<CDCTrack>& tracks) override;

    private:
      // Findlets
      /// Matcher for creating relations between tracks and segments based on the number of shared hits
      SharingHitsMatcher<CDCTrack, CDCSegment2D> m_sharedHitsMatcher;

      /// Select only those pairs, which share at least a certain amount of hits
      CutSelector<CDCTrack, CDCSegment2D> m_selectPairsWithSharedHits;

      /// Reference to the chooser to be used for matching segments and tracks in the first step.
      FilterSelector<CDCTrack, CDCSegment2D, ChooseableFilter<SegmentTrackFilterFactory>> m_chooseableSegmentTrackSelector;

      /// Select only the best matching segment-track relations and remove the hits from the other ones
      SingleMatchSelector<CDCTrack, CDCSegment2D> m_singleMatchSelector;

      /// Add the matched segments to tracks
      SegmentTrackAdderWithNormalization m_segmentTrackAdderWithNormalization;

      /// Findlet to filter out fake tracks
      TrackRejecter m_trackRejecter;

      /// Findlet for normalizing the tracks
      TrackNormalizer m_trackNormalizer;

      // Object pools
      std::vector<WeightedRelation<CDCTrack, const CDCSegment2D>> m_relations;
    };
  }
}
