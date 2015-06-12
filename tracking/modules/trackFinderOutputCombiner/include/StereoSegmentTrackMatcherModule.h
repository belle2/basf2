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
#include <tracking/trackFindingCDC/filters/segment_track/SimpleSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/Lookups.h>

#include <set>

namespace Belle2 {

  namespace TrackFindingCDC {
    /// Forward declarations
    class CDCRecoSegment2D;
    class CDCTrack;

    /// Forward declaration of the module implementing the segment track combiner based on various filters
    template < class SegmentTrackChooser = TrackFindingCDC::BaseSegmentTrackChooser> class StereoSegmentTrackMatcherModuleImpl;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::StereoSegmentTrackMatcherModuleImpl <TrackFindingCDC::SimpleSegmentTrackChooser>
  StereoSegmentTrackMatcherModule;

  namespace TrackFindingCDC {
    /// Matches the found stereo segments from the local track finder to the found tracks from the legendre track finder after stereo assignment.
    /// It uses hit based information - that means it matches a segment and a track if they both share hits and if the matching combination is unique.
    template <class SegmentTrackChooser>
    class StereoSegmentTrackMatcherModuleImpl : public TrackFinderCDCFromSegmentsModule {

    public:

      /**
       * Constructor to set the module parameters.
       */
      StereoSegmentTrackMatcherModuleImpl() : TrackFinderCDCFromSegmentsModule(), m_hitSegmentLookUp()
      {
        setDescription("Matches the found stereo segments from the local track finder to the found tracks from the legendre track finder after stereo assignment.\n"
                       "It uses hit based information - that means it matches a segment and a track if they both share hits and if the matching combination is unique.");
      }

      /** Initialize the filter */
      void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();

        if (m_ptrSegmentTrackChooser) {
          m_ptrSegmentTrackChooser->initialize();
        }
      }

      /** Terminate the filter */
      void terminate() override
      {
        TrackFinderCDCFromSegmentsModule::terminate();

        if (m_ptrSegmentTrackChooser) {
          m_ptrSegmentTrackChooser->terminate();
        }
      }

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

      /** Call the filter */
      bool segmentMatchesToTrack(const CDCRecoSegment2D* segment, const CDCTrack& track);

      /**
       * Find the matching segment to a reco hit
       */
      const TrackFindingCDC::CDCRecoSegment2D* findMatchingSegment(const TrackFindingCDC::CDCRecoHit3D& recoHit);

      /// Object pools

      /// A map from the found CDCHits to the RecoSegments. Uses pointers to the elements given to generate ( = elements in the StoreVector)
      std::map<const CDCHit*, const TrackFindingCDC::CDCRecoSegment2D*> m_hitSegmentLookUp;

      /// A map from the found Segments to the CDCTracks. Uses pointers to the elements given to generate ( = elements in the StoreVector)
      std::map<const TrackFindingCDC::CDCRecoSegment2D*, std::set<TrackFindingCDC::CDCTrack*>> m_segmentTrackLookUp;

      /// Reference to the chooser to be used for matching segments and tracks
      std::unique_ptr<SegmentTrackChooser> m_ptrSegmentTrackChooser;
    };


    template <class SegmentTrackChooser>
    void StereoSegmentTrackMatcherModuleImpl<SegmentTrackChooser>::fillHitLookUp(std::vector<CDCRecoSegment2D>& segments)
    {
      m_hitSegmentLookUp.clear();
      for (const CDCRecoSegment2D& segment : segments) {
        if (segment.getAutomatonCell().hasTakenFlag())
          continue;
        for (const CDCRecoHit2D& recoHit : segment) {
          const CDCHit* cdcHit = recoHit.getWireHit().getHit();
          m_hitSegmentLookUp.insert(std::make_pair(cdcHit, &segment));
        }
      }
    }

    template <class SegmentTrackChooser>
    const CDCRecoSegment2D* StereoSegmentTrackMatcherModuleImpl<SegmentTrackChooser>::findMatchingSegment(const CDCRecoHit3D& recoHit)
    {
      const CDCHit* cdcHit = recoHit.getWireHit().getHit();
      auto foundElement = m_hitSegmentLookUp.find(cdcHit);
      if (foundElement == m_hitSegmentLookUp.end()) {
        return nullptr;
      } else {
        return foundElement->second;
      }
    }

    template <class SegmentTrackChooser>
    bool StereoSegmentTrackMatcherModuleImpl<SegmentTrackChooser>::segmentMatchesToTrack(const CDCRecoSegment2D* segment,
        const CDCTrack& track)
    {
      if (TrackFindingCDC::isNotACell((*m_ptrSegmentTrackChooser)(std::make_pair(segment, &track)))) {
        return false;
      } else {
        return true;
      }
    }

    template <class SegmentTrackChooser>
    void StereoSegmentTrackMatcherModuleImpl<SegmentTrackChooser>::fillTrackLookUp(std::vector<CDCRecoSegment2D>& segments,
        std::vector<CDCTrack>& tracks)
    {
      // prepare lookup
      m_segmentTrackLookUp.clear();
      for (const CDCRecoSegment2D& segment : segments) {
        if (segment.getAutomatonCell().hasTakenFlag())
          continue;
        m_segmentTrackLookUp.insert(std::make_pair(&segment, std::set<TrackFindingCDC::CDCTrack*>()));
      }

      for (CDCTrack& track : tracks) {
        for (const CDCRecoHit3D& recoHit : track) {
          const CDCRecoSegment2D* matchingSegment = findMatchingSegment(recoHit);
          if (matchingSegment != nullptr and segmentMatchesToTrack(matchingSegment, track)) {
            m_segmentTrackLookUp[matchingSegment].insert(&track);
          }
        }
      }
    }

    template <class SegmentTrackChooser>
    void StereoSegmentTrackMatcherModuleImpl<SegmentTrackChooser>::generate(std::vector<CDCRecoSegment2D>& segments,
        std::vector<CDCTrack>& tracks)
    {
      // Mark the segments which are fully found by the legendre track finder as taken
      for (const CDCRecoSegment2D& segment : segments) {
        bool oneHitDoesNotHaveTakenFlag = false;
        for (const CDCRecoHit2D& recoHit : segment) {
          if (not recoHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
            oneHitDoesNotHaveTakenFlag = true;
            break;
          }
        }

        if (not oneHitDoesNotHaveTakenFlag) {
          segment.getAutomatonCell().setTakenFlag();
        }
      }

      fillHitLookUp(segments);
      fillTrackLookUp(segments, tracks);

      for (auto& segmentTrackCombination : m_segmentTrackLookUp) {
        const CDCRecoSegment2D* segment = segmentTrackCombination.first;
        const std::set<TrackFindingCDC::CDCTrack*>& matchingTracks = segmentTrackCombination.second;
        if (matchingTracks.size() == 1) {
          CDCTrack* track = *(matchingTracks.begin());
          SegmentTrackCombiner::addSegmentToTrack(*segment, *track);
        }
      }

      for (CDCTrack& track : tracks) {
        // Refit?
        track.sort();
      }
    }
  }
}
