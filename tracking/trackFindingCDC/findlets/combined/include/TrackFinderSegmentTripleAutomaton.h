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
#include <tracking/trackFindingCDC/findlets/minimal/TrackMerger.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackOrienter.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet implementing the track finding from segments using a cellular automaton over segment triples
    template < class AAxialSegmentPairFilter,
               class ASegmentTripleFilter,
               class ASegmentTripleRelationFilter,
               class ATrackRelationFilter>
    class TrackFinderSegmentTripleAutomaton
      : public Findlet<const CDCRecoSegment2D, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCRecoSegment2D, CDCTrack>;

    public:
      /// Default constructor initialising the filters with the default settings
      TrackFinderSegmentTripleAutomaton()
      {
        addProcessingSignalListener(&m_axialSegmentPairCreator);
        addProcessingSignalListener(&m_segmentTripleCreator);
        addProcessingSignalListener(&m_segmentTripleRelationCreator);
        addProcessingSignalListener(&m_trackCreatorSegmentTripleAutomaton);
        addProcessingSignalListener(&m_trackCreatorSingleSegments);
        addProcessingSignalListener(&m_trackMerger);
        addProcessingSignalListener(&m_trackOrienter);
        addProcessingSignalListener(&m_segmentTripleSwapper);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Generates tracks from segments using a cellular automaton build from segment triples.";
      }

      /// Expose the parameters of the cluster filter to a module
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        m_axialSegmentPairCreator.exposeParameters(moduleParamList, prefixed(prefix, "axialSegmentPair"));
        m_segmentTripleCreator.exposeParameters(moduleParamList, prefixed(prefix, "segmentTriple"));
        m_segmentTripleRelationCreator.exposeParameters(moduleParamList, prefixed(prefix, "segmentTripleRelation"));
        m_trackCreatorSegmentTripleAutomaton.exposeParameters(moduleParamList, prefix);
        m_trackCreatorSingleSegments.exposeParameters(moduleParamList, prefix);
        m_trackMerger.exposeParameters(moduleParamList, prefixed(prefix, "TrackRelation"));
        m_trackOrienter.exposeParameters(moduleParamList, prefix);
        m_segmentTripleSwapper.exposeParameters(moduleParamList, prefix);
      }

      ///  Initialize the Findlet before event processing
      virtual void beginEvent() override
      {
        m_axialSegmentPairs.clear();
        m_segmentTriples.clear();
        m_segmentTripleRelations.clear();
        m_preMergeTracks.clear();
        m_orientedTracks.clear();
        Super::beginEvent();
      }

      /// Generates the tracks from the given segments into the output argument.
      virtual void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                         std::vector<CDCTrack>& tracks) override final
      {
        m_axialSegmentPairCreator.apply(inputSegments, m_axialSegmentPairs);
        m_segmentTripleCreator.apply(inputSegments, m_axialSegmentPairs, m_segmentTriples);
        m_segmentTripleRelationCreator.apply(m_segmentTriples, m_segmentTripleRelations);
        m_trackCreatorSegmentTripleAutomaton.apply(m_segmentTriples, m_segmentTripleRelations, m_preMergeTracks);

        m_trackCreatorSingleSegments.apply(inputSegments, m_preMergeTracks);
        m_trackOrienter.apply(m_preMergeTracks, m_orientedTracks);
        m_trackMerger.apply(m_orientedTracks, tracks);

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
      WeightedRelationCreator<const CDCSegmentTriple,
                              ASegmentTripleRelationFilter> m_segmentTripleRelationCreator;

      /// Instance of the cellular automaton creating  creating tracks over segment triple
      TrackCreatorSegmentTripleAutomaton m_trackCreatorSegmentTripleAutomaton;

      /// Creates tracks from left over segments
      TrackCreatorSingleSegments m_trackCreatorSingleSegments;

      /// Findlet responsible for the merging of tracks
      TrackMerger<ATrackRelationFilter> m_trackMerger;

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

      /// Memory for the tracks before merging was applied.
      std::vector<CDCTrack> m_preMergeTracks;

      /// Memory for the tracks after orientation was applied.
      std::vector<CDCTrack> m_orientedTracks;
    };
  }
}
