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

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/ca/WeightedRelation.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    /// Links segments in the same super cluster by linking paths of segments in a cellular automaton
    template <class ASegmentRelationFilter>
    class SegmentLinker : public Findlet<const CDCSegment2D, CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCSegment2D>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentLinker()
      {
        this->addProcessingSignalListener(&m_segment2DRelationCreator);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Links segments by extraction of segment paths in a cellular automaton.";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_segment2DRelationCreator.exposeParameters(moduleParamList, prefix);
      }

    public:
      /// Main algorithm
      void apply(const std::vector<CDCSegment2D>& inputSegment2Ds,
                 std::vector<CDCSegment2D>& outputSegment2Ds) final {
        // Create linking relations
        m_segment2DRelations.clear();
        m_segment2DRelationCreator.apply(inputSegment2Ds, m_segment2DRelations);

        // Find linking paths
        m_segment2DPaths.clear();
        WeightedNeighborhood<const CDCSegment2D> segment2DNeighborhood(m_segment2DRelations);
        m_cellularPathFinder.apply(inputSegment2Ds, segment2DNeighborhood, m_segment2DPaths);

        // Put the linked segments together
        for (const Path<const CDCSegment2D>& segment2DPath : m_segment2DPaths)
        {
          outputSegment2Ds.push_back(CDCSegment2D::condense(segment2DPath));
        }
      }

    private:
      /// Creator of the segment relations for linking
      WeightedRelationCreator<const CDCSegment2D, ASegmentRelationFilter> m_segment2DRelationCreator;

      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCSegment2D> m_cellularPathFinder;

      /// Memory for the relations between segments to be followed on linking
      std::vector<WeightedRelation<const CDCSegment2D> > m_segment2DRelations;

      /// Memory for the segment paths generated from the graph.
      std::vector< Path<const CDCSegment2D> > m_segment2DPaths;
    };
  }
}
