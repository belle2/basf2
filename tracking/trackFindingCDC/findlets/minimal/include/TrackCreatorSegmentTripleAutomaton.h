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
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <iterator>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet that generates tracks based on a cellular automaton of segment triples
    class TrackCreatorSegmentTripleAutomaton
      : public Findlet<const CDCSegmentTriple,
        const WeightedRelation<const CDCSegmentTriple>,
        CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegmentTriple,
            const WeightedRelation<const CDCSegmentTriple>,
            CDCTrack>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Constructs tracks by extraction of segment triple paths in a cellular automaton.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList*,
                            const std::string& = "") override final
      {
      }

      /// Main function of the segment finding by the cellular automaton.
      virtual void apply(const std::vector<CDCSegmentTriple>& inputSegmentTriples,
                         const std::vector<WeightedRelation<const CDCSegmentTriple> >& inputSegmentTripleRelations,
                         std::vector<CDCTrack>& outputTracks) override final
      {
        m_segmentTriplePaths.clear();
        m_cellularPathFinder.apply(inputSegmentTriples,
                                   WeightedNeighborhood<const CDCSegmentTriple>(inputSegmentTripleRelations),
                                   m_segmentTriplePaths);
        B2DEBUG(100, "  Created " << m_segmentTriplePaths.size()  << " SegmentTripleTracks");

        // Reduce to plain tracks
        for (const Path<const CDCSegmentTriple>& segmentTriplePath : m_segmentTriplePaths) {
          outputTracks.push_back(CDCTrack::condense(segmentTriplePath));
        }
      }

    private:
      // object pools
      /// Memory for the segmentTriple paths generated from the graph.
      std::vector< Path<const CDCSegmentTriple> > m_segmentTriplePaths;

    private:
      // cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCSegmentTriple> m_cellularPathFinder;

    };

  }
}
