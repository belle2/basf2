/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackingUtilities/ca/MultipassCellularPathFinder.h>
#include <tracking/trackingUtilities/ca/Path.h>

#include <tracking/trackingUtilities/utilities/WeightedRelation.h>

#include <vector>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackingUtilities {
    class CDCTrack;
    class CDCSegmentTriple;
  }

  namespace TrackFindingCDC {

    /// Findlet that generates tracks based on a cellular automaton of segment triples
    class TrackCreatorSegmentTripleAutomaton
      : public TrackingUtilities::Findlet<const TrackingUtilities::CDCSegmentTriple, const TrackingUtilities::WeightedRelation<const TrackingUtilities::CDCSegmentTriple>, TrackingUtilities::CDCTrack> {

    private:
      /// Type of the base class
      using Super =
        TrackingUtilities::Findlet<const TrackingUtilities::CDCSegmentTriple, const TrackingUtilities::WeightedRelation<const TrackingUtilities::CDCSegmentTriple>, TrackingUtilities::CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main function of the segment finding by the cellular automaton.
      void apply(const std::vector<TrackingUtilities::CDCSegmentTriple>& inputSegmentTriples,
                 const std::vector<TrackingUtilities::WeightedRelation<const TrackingUtilities::CDCSegmentTriple>>& inputSegmentTripleRelations,
                 std::vector<TrackingUtilities::CDCTrack>& outputTracks) final;

    private: // cellular automaton
      /// Instance of the cellular automaton path finder
      TrackingUtilities::MultipassCellularPathFinder<const TrackingUtilities::CDCSegmentTriple> m_cellularPathFinder;

    private: // object pools
      /// Memory for the segment triple paths generated from the graph.
      std::vector<TrackingUtilities::Path<const TrackingUtilities::CDCSegmentTriple>> m_segmentTriplePaths;
    };
  }
}
