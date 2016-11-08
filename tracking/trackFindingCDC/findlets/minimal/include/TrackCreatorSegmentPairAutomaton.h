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

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/ca/Path.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <iterator>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet that generates tracks based on a cellular automaton of segment pairs
    class TrackCreatorSegmentPairAutomaton
      : public Findlet<const CDCSegmentPair,
        const WeightedRelation<const CDCSegmentPair>,
        CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegmentPair,
            const WeightedRelation<const CDCSegmentPair>,
            CDCTrack>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Constructs tracks by extraction of segment pair paths in a cellular automaton.";
      }

      /// Main function of the segment finding by the cellular automaton.
      virtual void apply(const std::vector<CDCSegmentPair>& inputSegmentPairs,
                         const std::vector<WeightedRelation<const CDCSegmentPair> >& inputSegmentPairRelations,
                         std::vector<CDCTrack>& outputTracks) override final
      {
        // Multiple passes if growMany is active and one track is created at a time
        // No best candidate analysis needed
        m_segmentPairPaths.clear();

        m_cellularPathFinder.apply(inputSegmentPairs,
                                   WeightedNeighborhood<const CDCSegmentPair>(inputSegmentPairRelations),
                                   m_segmentPairPaths);

        B2DEBUG(100, "  Created " << m_segmentPairPaths.size()  << " SegmentTripleTracks");

        // Reduce to plain tracks
        for (const Path<const CDCSegmentPair>& segmentPairPath : m_segmentPairPaths) {
          outputTracks.push_back(CDCTrack::condense(segmentPairPath));
          for (const CDCSegmentPair* segmentPair : segmentPairPath) {
            segmentPair->getFromSegment()->getAutomatonCell().setTakenFlag();
            segmentPair->getToSegment()->getAutomatonCell().setTakenFlag();
          }
        }
      }

    private:
      // object pools
      /// Memory for the segmentPair paths generated from the graph.
      std::vector< Path<const CDCSegmentPair> > m_segmentPairPaths;

    private:
      //cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCSegmentPair> m_cellularPathFinder;
    };


  }
}
