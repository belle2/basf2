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
#include <tracking/trackFindingCDC/filters/wirehit_relation/WholeWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <iterator>
#include <assert.h>

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
      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList) override final
      {
        m_segmentRelationFilter.exposeParameters(moduleParamList);
      }

      /// Signals the beginning of the event processing
      void initialize() override
      {
        Super::initialize();
        m_segmentRelationFilter.initialize();
      }

      /// Signals the beginning of a new event
      void beginEvent() override
      {
        Super::beginEvent();
        m_segmentRelationFilter.beginEvent();
      }

      /// Signals the end of the event processing
      void terminate() override
      {
        m_segmentRelationFilter.terminate();
        Super::terminate();
      }

    public:
      /// Main algorithm applying the cluster refinement
      virtual void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final
      {
        auto getISuperCluster = [](const CDCRecoSegment2D & segment) -> int { return segment.getISuperCluster();};
        std::vector<ConstVectorRange<CDCRecoSegment2D> > segmentsByISuperCluster =
          adjacent_groupby(inputSegments.begin(), inputSegments.end(), getISuperCluster);

        for (const ConstVectorRange<CDCRecoSegment2D>& segmentsInSuperCluster : segmentsByISuperCluster) {

          m_symmetricSegmentsInSuperCluster.clear();
          m_symmetricSegmentsInSuperCluster.reserve(2 * segmentsInSuperCluster.size());

          for (const CDCRecoSegment2D& segment : segmentsInSuperCluster) {
            m_symmetricSegmentsInSuperCluster.push_back(segment);
            m_symmetricSegmentsInSuperCluster.push_back(segment.reversed());
          }

          m_segmentNeighborhood.clear();
          m_segmentNeighborhood.createUsing(m_segmentRelationFilter,
                                            m_symmetricSegmentsInSuperCluster);

          m_segmentPaths.clear();
          m_cellularPathFinder.apply(m_symmetricSegmentsInSuperCluster,
                                     m_segmentNeighborhood,
                                     m_segmentPaths);

          for (const std::vector<const CDCRecoSegment2D*>& segmentPath : m_segmentPaths) {
            outputSegments.push_back(CDCRecoSegment2D::condense(segmentPath));
          }
        } // end super cluster loop
      }

    private:
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<CDCRecoSegment2D> m_cellularPathFinder;

      /// Memory for the symmetrised segments
      std::vector<CDCRecoSegment2D> m_symmetricSegmentsInSuperCluster;

      /// Memory for the wire hit neighborhood in a cluster.
      WeightedNeighborhood<const CDCRecoSegment2D> m_segmentNeighborhood;

      /// Memory for the segment paths generated from the graph.
      std::vector< std::vector<const CDCRecoSegment2D*> > m_segmentPaths;

      /// Wire hit neighborhood relation filter
      SegmentRelationFilter m_segmentRelationFilter;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
