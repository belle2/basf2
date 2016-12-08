/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/AxialSegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentTripleCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/WeightedRelationCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentTripleAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackOrienter.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/ChooseableAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/ChooseableSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/ChooseableSegmentTripleRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet implementing the track finding from segments using a cellular automaton over segment triples
    class TrackFinderSegmentTripleAutomaton : public Findlet<const CDCSegment2D, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCTrack>;

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
      void apply(const std::vector<CDCSegment2D>& inputSegments, std::vector<CDCTrack>& tracks) final;

    private:
      // Findlets
      /// Instance of the axial to axial segment pair creator
      AxialSegmentPairCreator<ChooseableAxialSegmentPairFilter> m_axialSegmentPairCreator;

      /// Instance of the segment triple creator
      SegmentTripleCreator<ChooseableSegmentTripleFilter> m_segmentTripleCreator;

      /// Instance of the segment triple relation creator
      WeightedRelationCreator<const CDCSegmentTriple, ChooseableSegmentTripleRelationFilter> m_segmentTripleRelationCreator;

      /// Instance of the cellular automaton creating  creating tracks over segment triple
      TrackCreatorSegmentTripleAutomaton m_trackCreatorSegmentTripleAutomaton;

      /// Creates tracks from left over segments
      TrackCreatorSingleSegments m_trackCreatorSingleSegments;

      /// Findlet responsible for the linking of tracks
      TrackLinker m_trackLinker;

      /// Fixes the direction of flight of tracks by a simple chooseable heuristic.
      TrackOrienter m_trackOrienter;

      /// Helper to swap the local segment triples out to the DataStore
      StoreVectorSwapper<CDCSegmentTriple> m_segmentTripleSwapper{"CDCSegmentTripleVector"};

      // object pools
      /// Memory for the axial to axial segment pairs
      std::vector<CDCAxialSegmentPair> m_axialSegmentPairs;

      /// Memory for the segment triples
      std::vector<CDCSegmentTriple> m_segmentTriples;

      /// Memory for the segment triple relations
      std::vector<WeightedRelation<const CDCSegmentTriple> > m_segmentTripleRelations;

      /// Memory for the tracks before linking was applied.
      std::vector<CDCTrack> m_preLinkingTracks;

      /// Memory for the tracks after orientation was applied.
      std::vector<CDCTrack> m_orientedTracks;
    };
  }
}
