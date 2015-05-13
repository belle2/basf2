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
#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/SimpleSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrainFilter.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
  }

  namespace TrackFinderOutputCombining {

    /// Forward declaration of the module implementing the segment track combiner based on various filters
    template < class SegmentTrackChooser = TrackFindingCDC::BaseSegmentTrackChooser,
               class SegmentTrainFilter = TrackFindingCDC::BaseSegmentTrainFilter,
               class SegmentTrackFilter  = TrackFindingCDC::BaseSegmentTrackFilter>
    class SegmentTrackCombinerImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFinderOutputCombining::SegmentTrackCombinerImplModule <
  TrackFindingCDC::SimpleSegmentTrackChooser,
                  TrackFindingCDC::BaseSegmentTrainFilter,
                  TrackFindingCDC::BaseSegmentTrackFilter
                  > SegmentTrackCombinerModule;

  namespace TrackFinderOutputCombining {
    template < class SegmentTrackChooser,
               class SegmentTrainFilter,
               class SegmentTrackFilter>
    class SegmentTrackCombinerImplModule : public TrackFinderCDCFromSegmentsModule {

    public:
      /**
       * Constructor.
       */
      SegmentTrackCombinerImplModule(): TrackFinderCDCFromSegmentsModule(),
        m_combiner(),
        m_ptrSegmentTrackChooser(new SegmentTrackChooser),
        m_ptrSegmentTrainFilter(new SegmentTrainFilter),
        m_ptrSegmentTrackFilter(new SegmentTrackFilter) { }

      /** Initialize the filter */
      void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();

        if (m_ptrSegmentTrackChooser) {
          m_ptrSegmentTrackChooser->initialize();
        }

        if (m_ptrSegmentTrainFilter) {
          m_ptrSegmentTrainFilter->initialize();
        }

        if (m_ptrSegmentTrackFilter) {
          m_ptrSegmentTrackFilter->initialize();
        }
      }

      /** Terminate the filter */
      void terminate() override
      {
        TrackFinderCDCFromSegmentsModule::terminate();

        if (m_ptrSegmentTrackChooser) {
          m_ptrSegmentTrackChooser->terminate();
        }

        if (m_ptrSegmentTrainFilter) {
          m_ptrSegmentTrainFilter->terminate();
        }

        if (m_ptrSegmentTrackFilter) {
          m_ptrSegmentTrackFilter->terminate();
        }
      }

    private:
      /**
       * Try to combine the segments and the tracks
       */
      void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

    public:
      /// Getter for the current SegmentTrackChooser. The module keeps ownership of the pointer.
      SegmentTrackChooser* getSegmentTrackChooser()
      {
        return m_ptrSegmentTrackChooser.get();
      }

      /// Setter for the SegmentTrackChooser. The module takes ownership of the pointer.
      void setSegmentTrackChooser(std::unique_ptr<SegmentTrackChooser> ptrClusterFilter)
      {
        m_ptrSegmentTrackChooser = std::move(ptrClusterFilter);
      }

      /// Getter for the current SegmentTrainFilter. The module keeps ownership of the pointer.
      SegmentTrainFilter* getSegmentTrainFilter()
      {
        return m_ptrSegmentTrainFilter.get();
      }

      /// Setter for the SegmentTrainFilter. The module takes ownership of the pointer.
      void setSegmentTrainFilter(std::unique_ptr<SegmentTrainFilter> ptrFacetFilter)
      {
        m_ptrSegmentTrainFilter = std::move(ptrFacetFilter);
      }

      /// Getter for the current SegmentTrackFilter. The module keeps ownership of the pointer.
      SegmentTrackFilter* getSegmentTrackFilter()
      {
        return m_ptrSegmentTrackFilter.get();
      }

      /// Setter for the SegmentTrackFilter. The module takes ownership of the pointer.
      void setSegmentTrackFilter(std::unique_ptr<SegmentTrackFilter> ptrFacetNeighborChooser)
      {
        m_ptrSegmentTrackFilter = std::move(ptrFacetNeighborChooser);
      }

    private:
      /// Object that handles the combination
      TrackFinderOutputCombining::SegmentTrackCombiner m_combiner;

      /// Reference to the chooser to be used for matching segments and tracks
      std::unique_ptr<SegmentTrackChooser> m_ptrSegmentTrackChooser;

      /// Reference to the filter to be used for construction segment trains
      std::unique_ptr<SegmentTrainFilter> m_ptrSegmentTrainFilter;

      /// Reference to the filter to be used to do an uniqe segment train <-> track matching
      std::unique_ptr<SegmentTrackFilter> m_ptrSegmentTrackFilter;
    };

    template < class SegmentTrackChooser,
               class SegmentTrainFilter,
               class SegmentTrackFilter>
    void SegmentTrackCombinerImplModule<SegmentTrackChooser, SegmentTrainFilter, SegmentTrackFilter>::generate(
      std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks)
    {
      m_combiner.fillWith(tracks, segments);
      m_combiner.combine(*m_ptrSegmentTrackChooser, *m_ptrSegmentTrainFilter, *m_ptrSegmentTrackFilter);

      // Delete all used segments
      segments.erase(std::remove_if(segments.begin(), segments.end(), [](const TrackFindingCDC::CDCRecoSegment2D & segment) -> bool {
        return segment.getAutomatonCell().hasTakenFlag();
      }), segments.end());

      B2WARNING("After all there are " << segments.size() << " Segments left in this event.")

      // Reset the taken flag for the hits of all the unused segments
      for (const TrackFindingCDC::CDCRecoSegment2D& segment : segments) {
        for (const TrackFindingCDC::CDCRecoHit2D& recoHit : segment) {
          recoHit.getWireHit().getAutomatonCell().unsetTakenFlag();
        }
      }

      m_combiner.clear();
    }
  }
}
