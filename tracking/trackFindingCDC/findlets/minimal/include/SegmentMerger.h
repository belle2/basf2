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
#include <tracking/trackFindingCDC/filters/wireHitRelation/WholeWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <iterator>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Merges segments in the same super cluster by linking paths of segments in a cellular automaton
    template<class SegmentRelationFilter>
    class SegmentMerger:
      public Findlet<const CDCRecoSegment2D, CDCRecoSegment2D> {

    private:
      /// Type of the base class
      typedef Findlet<const CDCRecoSegment2D, CDCRecoSegment2D> Super;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentMerger()
      {
        addProcessingSignalListener(&m_segmentRelationFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Merges segments by extraction of segment paths in a cellular automaton.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final
      {
        m_segmentRelationFilter.exposeParameters(moduleParamList, prefix);
      }

    public:
      /// Main algorithm
      virtual void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final
      {
        auto getISuperCluster = [](const CDCRecoSegment2D & segment) -> int { return segment.getISuperCluster();};
        std::vector<ConstVectorRange<CDCRecoSegment2D> > segmentsByISuperCluster =
          adjacent_groupby(inputSegments.begin(), inputSegments.end(), getISuperCluster);

        for (const ConstVectorRange<CDCRecoSegment2D>& segmentsInSuperCluster : segmentsByISuperCluster) {
          if (segmentsInSuperCluster.size() == 1) {
            // What is the difference between a duck?
            outputSegments.push_back(segmentsInSuperCluster.at(0));
            continue;
          }

          m_symmetricSegmentsInSuperCluster.clear();
          m_symmetricSegmentsInSuperCluster.reserve(2 * segmentsInSuperCluster.size());

          for (const CDCRecoSegment2D& segment : segmentsInSuperCluster) {
            m_symmetricSegmentsInSuperCluster.push_back(segment);
            m_symmetricSegmentsInSuperCluster.push_back(segment.reversed());
          }

          m_segmentRelations.clear();
          WeightedNeighborhood<const CDCRecoSegment2D>::appendUsing(m_segmentRelationFilter,
                                                                    m_symmetricSegmentsInSuperCluster,
                                                                    m_segmentRelations);
          WeightedNeighborhood<const CDCRecoSegment2D> segmentNeighborhood(m_segmentRelations);

          m_segmentPaths.clear();
          m_cellularPathFinder.apply(m_symmetricSegmentsInSuperCluster,
                                     segmentNeighborhood,
                                     m_segmentPaths);

          for (const std::vector<const CDCRecoSegment2D*>& segmentPath : m_segmentPaths) {
            outputSegments.push_back(CDCRecoSegment2D::condense(segmentPath));
          }
        } // end super cluster loop
      }

    private:
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCRecoSegment2D> m_cellularPathFinder;

      /// Memory for the symmetrised segments
      std::vector<CDCRecoSegment2D> m_symmetricSegmentsInSuperCluster;

      /// Memory for the relations between segments to be followed on merge
      std::vector<WeightedRelation<const CDCRecoSegment2D> > m_segmentRelations;

      /// Memory for the segment paths generated from the graph.
      std::vector< std::vector<const CDCRecoSegment2D*> > m_segmentPaths;

      /// Wire hit neighborhood relation filter
      SegmentRelationFilter m_segmentRelationFilter;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
