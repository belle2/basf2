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

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    /// Links segments in the same super cluster by linking paths of segments in a cellular automaton
    template <class SegmentRelationFilter>
    class SegmentLinker : public Findlet<const CDCSegment2D, CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCSegment2D>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentLinker()
      {
        this->addProcessingSignalListener(&m_segmentRelationFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Links segments by extraction of segment paths in a cellular automaton.";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_segmentRelationFilter.exposeParameters(moduleParamList, prefix);
      }

    public:
      /// Main algorithm
      void apply(const std::vector<CDCSegment2D>& inputSegments,
                 std::vector<CDCSegment2D>& outputSegments) final {
        m_segmentRelations.clear();
        WeightedNeighborhood<const CDCSegment2D>::appendUsing(m_segmentRelationFilter,
        inputSegments,
        m_segmentRelations);
        WeightedNeighborhood<const CDCSegment2D> segmentNeighborhood(m_segmentRelations);

        m_segmentPaths.clear();
        m_cellularPathFinder.apply(inputSegments,
        segmentNeighborhood,
        m_segmentPaths);

        for (const Path<const CDCSegment2D>& segmentPath : m_segmentPaths)
        {
          outputSegments.push_back(CDCSegment2D::condense(segmentPath));
        }
      }

    private:
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCSegment2D> m_cellularPathFinder;

      /// Memory for the relations between segments to be followed on linking
      std::vector<WeightedRelation<const CDCSegment2D> > m_segmentRelations;

      /// Memory for the segment paths generated from the graph.
      std::vector< Path<const CDCSegment2D> > m_segmentPaths;

      /// Wire hit neighborhood relation filter
      SegmentRelationFilter m_segmentRelationFilter;
    };
  }
}
