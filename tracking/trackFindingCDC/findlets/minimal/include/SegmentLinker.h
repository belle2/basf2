/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>
#include <tracking/trackingUtilities/findlets/minimal/WeightedRelationCreator.h>

#include <tracking/trackFindingCDC/filters/segmentRelation/ChooseableSegmentRelationFilter.h>

#include <tracking/trackingUtilities/ca/MultipassCellularPathFinder.h>
#include <tracking/trackingUtilities/utilities/WeightedRelation.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCSegment2D;
  }
  namespace TrackFindingCDC {

    /// Links segments in the same super cluster by linking paths of segments in a cellular automaton
    class SegmentLinker : public TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D, TrackingUtilities::CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D, TrackingUtilities::CDCSegment2D>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentLinker();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm
      void apply(const std::vector<TrackingUtilities::CDCSegment2D>& inputSegment2Ds,
                 std::vector<TrackingUtilities::CDCSegment2D>& outputSegment2Ds) final;

    private: // Parameters
      /// Parameter : Switch to activate segment linking in the whole superlayer instead of only the super cluster
      bool m_param_wholeSuperLayer = false;

      /// Parameter : Switch to block hits that appear in linked segments such that unlinked reverse and aliases are excluded
      bool m_param_dealiasLinked = false;

      /// Parameter : Switch to construct only segments that have a linked partner
      bool m_param_onlyLinked = false;

    private:
      /// Creator of the segment relations for linking
      TrackingUtilities::WeightedRelationCreator<const TrackingUtilities::CDCSegment2D, TrackFindingCDC::ChooseableSegmentRelationFilter>
      m_segment2DRelationCreator;

      /// Instance of the cellular automaton path finder
      TrackingUtilities::MultipassCellularPathFinder<const TrackingUtilities::CDCSegment2D> m_cellularPathFinder;

      /// Memory for the relations between segments to be followed on linking
      std::vector<TrackingUtilities::WeightedRelation<const TrackingUtilities::CDCSegment2D>> m_segment2DRelations;

      /// Memory for the segment paths generated from the graph.
      std::vector<TrackingUtilities::Path<const TrackingUtilities::CDCSegment2D>> m_segment2DPaths;
    };
  }
}
