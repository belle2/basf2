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

#include <tracking/trackFindingCDC/findlets/minimal/SegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/WeightedRelationCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackMerger.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackOrienter.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Findlet implementing the track finding from segments using a cellular automaton over segment pairs
    template<class ASegmentPairFilter,
             class ASegmentPairRelationFilter,
             class ATrackRelationFilter>
    class TrackFinderSegmentPairAutomaton
      : public Findlet<const CDCRecoSegment2D, CDCTrack> {

    private:
      /// Type of the base class
      typedef Findlet<const CDCRecoSegment2D, CDCTrack> Super;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      TrackFinderSegmentPairAutomaton()
      {
        addProcessingSignalListener(&m_segmentPairCreator);
        addProcessingSignalListener(&m_segmentPairRelationCreator);
        addProcessingSignalListener(&m_trackCreatorSegmentPairAutomaton);
        addProcessingSignalListener(&m_trackCreatorSingleSegments);
        addProcessingSignalListener(&m_trackMerger);
        addProcessingSignalListener(&m_trackOrienter);
        addProcessingSignalListener(&m_segmentPairSwapper);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Generates tracks from segments using a cellular automaton build from segment pairs.";
      }

      /// Expose the parameters of the cluster filter to a module
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override
      {
        m_segmentPairCreator.exposeParameters(moduleParamList, prefixed(prefix, "SegmentPair"));
        m_segmentPairRelationCreator.exposeParameters(moduleParamList, prefixed(prefix, "SegmentPairRelation"));
        m_trackCreatorSegmentPairAutomaton.exposeParameters(moduleParamList, prefix);
        m_trackCreatorSingleSegments.exposeParameters(moduleParamList, prefix);
        m_trackMerger.exposeParameters(moduleParamList, prefixed(prefix, "TrackRelation"));
        m_trackOrienter.exposeParameters(moduleParamList, prefix);
        m_segmentPairSwapper.exposeParameters(moduleParamList, prefix);

        moduleParamList->getParameter<int>("SegmentPairRelationOnlyBest").setDefaultValue(1);
        moduleParamList->getParameter<int>("SegmentPairRelationOnlyBest").resetValue();
      }

      /// Signal the beginning of a new event the current event
      void beginEvent() override
      {
        m_segmentPairs.clear();
        m_segmentPairRelations.clear();
        m_preMergeTracks.clear();
        m_orientedTracks.clear();
        Super::beginEvent();
      }

      /// Generates the segment.
      virtual void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                         std::vector<CDCTrack>& tracks) override final
      {
        m_segmentPairs.reserve(100);
        m_segmentPairRelations.reserve(100);
        m_preMergeTracks.reserve(20);
        m_orientedTracks.reserve(20);

        m_segmentPairCreator.apply(inputSegments, m_segmentPairs);
        m_segmentPairRelationCreator.apply(m_segmentPairs, m_segmentPairRelations);
        m_trackCreatorSegmentPairAutomaton.apply(m_segmentPairs, m_segmentPairRelations, m_preMergeTracks);

        m_trackCreatorSingleSegments.apply(inputSegments, m_preMergeTracks);

        m_trackOrienter.apply(m_preMergeTracks, m_orientedTracks);
        m_trackMerger.apply(m_orientedTracks, tracks);

        // Put the segment pairs on the DataStore
        m_segmentPairSwapper.apply(m_segmentPairs);
      }

    private:
      // Findlets
      /// Findlet responsible for the creation of segment pairs
      SegmentPairCreator<ASegmentPairFilter> m_segmentPairCreator;

      /// Findlet responsible for the creation of segment pairs relations of the CA.
      WeightedRelationCreator<const CDCSegmentPair,
                              ASegmentPairRelationFilter> m_segmentPairRelationCreator;

      /// Reference to the relation filter to be used to construct the segment pair network.
      TrackCreatorSegmentPairAutomaton m_trackCreatorSegmentPairAutomaton;

      /// Creates tracks from left over segments
      TrackCreatorSingleSegments m_trackCreatorSingleSegments;

      /// Findlet responsible for the merging of tracks
      TrackMerger<ATrackRelationFilter> m_trackMerger;

      /// Fixes the direction of flight of tracks by a simple chooseable heuristic.
      TrackOrienter m_trackOrienter;

      /// Puts the internal segment pairs on the DataStore
      StoreVectorSwapper<CDCSegmentPair> m_segmentPairSwapper{"CDCSegmentPairVector"};

      // Object pools
      /// Memory for the axial stereo segment pairs.
      std::vector<CDCSegmentPair> m_segmentPairs;

      /// Memory for the axial stereo segment pair relations.
      std::vector<WeightedRelation<const CDCSegmentPair> > m_segmentPairRelations;

      /// Memory for the tracks before merging was applied.
      std::vector<CDCTrack> m_preMergeTracks;

      /// Memory for the tracks after orientation was applied.
      std::vector<CDCTrack> m_orientedTracks;

    }; // end class TrackFinderSegmentPairAutomaton

  } //end namespace TrackFindingCDC
} //end namespace Belle2
