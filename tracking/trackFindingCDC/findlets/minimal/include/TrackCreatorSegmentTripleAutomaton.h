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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/ca/Path.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <vector>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCSegmentTriple;
    class CDCTrack;

    /// Findlet that generates tracks based on a cellular automaton of segment triples
    class TrackCreatorSegmentTripleAutomaton
      : public Findlet<const CDCSegmentTriple, const WeightedRelation<const CDCSegmentTriple>, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegmentTriple, const WeightedRelation<const CDCSegmentTriple>, CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main function of the segment finding by the cellular automaton.
      void apply(const std::vector<CDCSegmentTriple>& inputSegmentTriples,
                 const std::vector<WeightedRelation<const CDCSegmentTriple>>& inputSegmentTripleRelations,
                 std::vector<CDCTrack>& outputTracks) final;

    private: // cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCSegmentTriple> m_cellularPathFinder;

    private: // object pools
      /// Memory for the segment triple paths generated from the graph.
      std::vector<Path<const CDCSegmentTriple>> m_segmentTriplePaths;
    };
  }
}
