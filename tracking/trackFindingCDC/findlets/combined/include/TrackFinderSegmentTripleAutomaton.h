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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet implementing the track finding from segments using a cellular automaton over segment triples
    template <class AAxialSegmentPairFilter,
              class ASegmentTripleFilter,
              class ASegmentTripleRelationFilter,
              class ATrackRelationFilter>
    class TrackFinderSegmentTripleAutomaton : public Findlet<const CDCSegment2D, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCTrack>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      TrackFinderSegmentTripleAutomaton()
      {
        this->addProcessingSignalListener(&m_axialSegmentPairCreator);
        this->addProcessingSignalListener(&m_segmentTripleCreator);
        this->addProcessingSignalListener(&m_segmentTripleRelationCreator);
        this->addProcessingSignalListener(&m_trackCreatorSegmentTripleAutomaton);
        this->addProcessingSignalListener(&m_trackCreatorSingleSegments);
        this->addProcessingSignalListener(&m_trackLinker);
        this->addProcessingSignalListener(&m_trackOrienter);
        this->addProcessingSignalListener(&m_segmentTripleSwapper);

        m_axialSegmentPairs.reserve(75);
        m_segmentTriples.reserve(100);
        m_segmentTripleRelations.reserve(100);
        m_preLinkingTracks.reserve(20);
        m_orientedTracks.reserve(20);
      }

      /// Short description of the findlet
      std::string getDescription() override final
      {
        return "Generates tracks from segments using a cellular automaton built from segment triples.";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final
      {
        m_axialSegmentPairCreator.exposeParameters(moduleParamList, prefixed(prefix, "axialSegmentPair"));
        m_segmentTripleCreator.exposeParameters(moduleParamList, prefixed(prefix, "segmentTriple"));
        m_segmentTripleRelationCreator.exposeParameters(moduleParamList, prefixed(prefix, "segmentTripleRelation"));
        m_trackCreatorSegmentTripleAutomaton.exposeParameters(moduleParamList, prefix);
        m_trackCreatorSingleSegments.exposeParameters(moduleParamList, prefix);
        m_trackLinker.exposeParameters(moduleParamList, prefixed(prefix, "TrackRelation"));
        m_trackOrienter.exposeParameters(moduleParamList, prefix);
        m_segmentTripleSwapper.exposeParameters(moduleParamList, prefix);
      }

      /// Signal the beginning of a new event
      void beginEvent() override final
      {
        m_axialSegmentPairs.clear();
        m_segmentTriples.clear();
        m_segmentTripleRelations.clear();
        m_preLinkingTracks.clear();
        m_orientedTracks.clear();
        Super::beginEvent();
      }

      /// Generates the tracks.
      void apply(const std::vector<CDCSegment2D>& inputSegments,
                 std::vector<CDCTrack>& tracks) override final
      {
        m_axialSegmentPairCreator.apply(inputSegments, m_axialSegmentPairs);
        m_segmentTripleCreator.apply(inputSegments, m_axialSegmentPairs, m_segmentTriples);
        m_segmentTripleRelationCreator.apply(m_segmentTriples, m_segmentTripleRelations);
        m_trackCreatorSegmentTripleAutomaton.apply(m_segmentTriples, m_segmentTripleRelations, m_preLinkingTracks);

        m_trackCreatorSingleSegments.apply(inputSegments, m_preLinkingTracks);
        m_trackOrienter.apply(m_preLinkingTracks, m_orientedTracks);
        m_trackLinker.apply(m_orientedTracks, tracks);

        // Put the segment triples on the DataStore
        m_segmentTripleSwapper.apply(m_segmentTriples);
      }

    private:
      // Findlets
      /// Instance of the axial to axial segment pair creator
      AxialSegmentPairCreator<AAxialSegmentPairFilter> m_axialSegmentPairCreator;

      /// Instance of the segment triple creator
      SegmentTripleCreator<ASegmentTripleFilter> m_segmentTripleCreator;

      /// Instance of the segment triple relation creator
      WeightedRelationCreator<const CDCSegmentTriple, ASegmentTripleRelationFilter> m_segmentTripleRelationCreator;

      /// Instance of the cellular automaton creating  creating tracks over segment triple
      TrackCreatorSegmentTripleAutomaton m_trackCreatorSegmentTripleAutomaton;

      /// Creates tracks from left over segments
      TrackCreatorSingleSegments m_trackCreatorSingleSegments;

      /// Findlet responsible for the linking of tracks
      TrackLinker<ATrackRelationFilter> m_trackLinker;

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
