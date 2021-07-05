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
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackOrienter.h>

#include <tracking/trackFindingCDC/filters/segmentPairRelation/ChooseableSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/findlets/minimal/WeightedRelationCreator.h>
#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Findlet implementing the track finding from segments using a cellular automaton over segment pairs
    class TrackFinderSegmentPairAutomaton : public Findlet<const CDCSegment2D, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCTrack>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      TrackFinderSegmentPairAutomaton();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signal the beginning of a new event
      void beginEvent() final;

      /// Generates the tracks from segments
      void apply(const std::vector<CDCSegment2D>& inputSegments, std::vector<CDCTrack>& tracks) final;

    private:
      // Findlets
      /// Findlet responsible for the creation of segment pairs
      SegmentPairCreator m_segmentPairCreator;

      /// Findlet responsible for the creation of segment pairs relations of the CA.
      WeightedRelationCreator<const CDCSegmentPair, ChooseableSegmentPairRelationFilter> m_segmentPairRelationCreator;

      /// Reference to the relation filter to be used to construct the segment pair network.
      TrackCreatorSegmentPairAutomaton m_trackCreatorSegmentPairAutomaton;

      /// Creates tracks from left over segments
      TrackCreatorSingleSegments m_trackCreatorSingleSegments;

      /// Findlet responsible for the linking of tracks
      TrackLinker m_trackLinker;

      /// Fixes the direction of flight of tracks by a simple chooseable heuristic.
      TrackOrienter m_trackOrienter;

      /// Puts the internal segment pairs on the DataStore
      StoreVectorSwapper<CDCSegmentPair> m_segmentPairSwapper{"CDCSegmentPairVector"};

      // Object pools
      /// Memory for the axial stereo segment pairs.
      std::vector<CDCSegmentPair> m_segmentPairs;

      /// Memory for the axial stereo segment pair relations.
      std::vector<WeightedRelation<const CDCSegmentPair> > m_segmentPairRelations;

      /// Memory for the tracks before linking was applied.
      std::vector<CDCTrack> m_preLinkingTracks;

      /// Memory for the tracks after orientation was applied.
      std::vector<CDCTrack> m_orientedTracks;
    };
  }
}
