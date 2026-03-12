/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/AxialSegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentTripleCreator.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentTripleAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackOrienter.h>

#include <tracking/trackFindingCDC/filters/segmentTripleRelation/ChooseableSegmentTripleRelationFilter.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>
#include <tracking/trackingUtilities/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackingUtilities/eventdata/tracks/CDCAxialSegmentPair.h>

#include <tracking/trackingUtilities/findlets/minimal/WeightedRelationCreator.h>
#include <tracking/trackingUtilities/findlets/base/StoreVectorSwapper.h>

#include <vector>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
  }
  namespace TrackFindingCDC {

    /// Findlet implementing the track finding from segments using a cellular automaton over segment triples
    class TrackFinderSegmentTripleAutomaton : public
      TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D, TrackingUtilities::CDCTrack> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D, TrackingUtilities::CDCTrack>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      TrackFinderSegmentTripleAutomaton();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signal the beginning of a new event
      void beginEvent() final;

      /// Generates the tracks.
      void apply(const std::vector<TrackingUtilities::CDCSegment2D>& inputSegments,
                 std::vector<TrackingUtilities::CDCTrack>& tracks) final;

    private:
      // Findlets
      /// Instance of the axial to axial segment pair creator
      AxialSegmentPairCreator m_axialSegmentPairCreator;

      /// Instance of the segment triple creator
      SegmentTripleCreator m_segmentTripleCreator;

      /// Instance of the segment triple relation creator
      TrackingUtilities::WeightedRelationCreator<const TrackingUtilities::CDCSegmentTriple, TrackFindingCDC::ChooseableSegmentTripleRelationFilter>
      m_segmentTripleRelationCreator;

      /// Instance of the cellular automaton creating  creating tracks over segment triple
      TrackCreatorSegmentTripleAutomaton m_trackCreatorSegmentTripleAutomaton;

      /// Creates tracks from left over segments
      TrackCreatorSingleSegments m_trackCreatorSingleSegments;

      /// Findlet responsible for the linking of tracks
      TrackLinker m_trackLinker;

      /// Fixes the direction of flight of tracks by a simple chooseable heuristic.
      TrackOrienter m_trackOrienter;

      /// Helper to swap the local segment triples out to the DataStore
      TrackingUtilities::StoreVectorSwapper<TrackingUtilities::CDCSegmentTriple> m_segmentTripleSwapper{"CDCSegmentTripleVector"};

      // object pools
      /// Memory for the axial to axial segment pairs
      std::vector<TrackingUtilities::CDCAxialSegmentPair> m_axialSegmentPairs;

      /// Memory for the segment triples
      std::vector<TrackingUtilities::CDCSegmentTriple> m_segmentTriples;

      /// Memory for the segment triple relations
      std::vector<TrackingUtilities::WeightedRelation<const TrackingUtilities::CDCSegmentTriple> > m_segmentTripleRelations;

      /// Memory for the tracks before linking was applied.
      std::vector<TrackingUtilities::CDCTrack> m_preLinkingTracks;

      /// Memory for the tracks after orientation was applied.
      std::vector<TrackingUtilities::CDCTrack> m_orientedTracks;
    };
  }
}
