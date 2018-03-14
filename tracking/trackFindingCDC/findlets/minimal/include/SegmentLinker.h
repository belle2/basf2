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
#include <tracking/trackFindingCDC/findlets/minimal/WeightedRelationCreator.h>

#include <tracking/trackFindingCDC/filters/segmentRelation/ChooseableSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCSegment2D;

    /// Links segments in the same super cluster by linking paths of segments in a cellular automaton
    class SegmentLinker : public Findlet<const CDCSegment2D, CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCSegment2D>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentLinker();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm
      void apply(const std::vector<CDCSegment2D>& inputSegment2Ds,
                 std::vector<CDCSegment2D>& outputSegment2Ds) final;

    private: // Parameters
      /// Parameter : Switch to activate segment linking in the whole superlayer instead of only the super cluster
      bool m_param_wholeSuperLayer = false;

      /// Parameter : Switch to block hits that appear in linked segments such that unlinked reverse and aliases are excluded
      bool m_param_dealiasLinked = false;

      /// Parameter : Switch to construct only segments that have a linked partner
      bool m_param_onlyLinked = false;

    private:
      /// Creator of the segment relations for linking
      WeightedRelationCreator<const CDCSegment2D, ChooseableSegmentRelationFilter> m_segment2DRelationCreator;

      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCSegment2D> m_cellularPathFinder;

      /// Memory for the relations between segments to be followed on linking
      std::vector<WeightedRelation<const CDCSegment2D>> m_segment2DRelations;

      /// Memory for the segment paths generated from the graph.
      std::vector<Path<const CDCSegment2D>> m_segment2DPaths;
    };
  }
}
