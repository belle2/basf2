/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackingUtilities/ca/MultipassCellularPathFinder.h>
#include <tracking/trackingUtilities/ca/Path.h>

#include <tracking/trackingUtilities/utilities/WeightedRelation.h>

#include <vector>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackingUtilities {
    class CDCTrack;
  }

  namespace TrackFindingCDC {
    class CDCSegmentPair;

    /// Findlet that generates tracks based on a cellular automaton of segment pairs
    class TrackCreatorSegmentPairAutomaton
      : public TrackingUtilities::Findlet<const CDCSegmentPair, const TrackingUtilities::WeightedRelation<const CDCSegmentPair>, TrackingUtilities::CDCTrack> {

    private:
      /// Type of the base class
      using Super =
        TrackingUtilities::Findlet<const CDCSegmentPair, const TrackingUtilities::WeightedRelation<const CDCSegmentPair>, TrackingUtilities::CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main function of the segment finding by the cellular automaton.
      void
      apply(const std::vector<CDCSegmentPair>& inputSegmentPairs,
            const std::vector<TrackingUtilities::WeightedRelation<const CDCSegmentPair>>& inputSegmentPairRelations,
            std::vector<TrackingUtilities::CDCTrack>& outputTracks) final;

    private: // cellular automaton
      /// Instance of the cellular automaton path finder
      TrackingUtilities::MultipassCellularPathFinder<const CDCSegmentPair> m_cellularPathFinder;

    private: // object pools
      /// Memory for the segment pair paths generated from the graph.
      std::vector<TrackingUtilities::Path<const CDCSegmentPair>> m_segmentPairPaths;
    };
  }
}
