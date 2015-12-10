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
        getSegmentRelationFilter().exposeParameters(moduleParamList);
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

      /// Getter for the contained segment relation filter
      SegmentRelationFilter& getSegmentRelationFilter()
      { return m_segmentRelationFilter; }

    public:
      /// Main algorithm applying the cluster refinement
      virtual void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final
      {
        std::vector<ConstVectorRange<CDCRecoSegment2D> > m_segmentsByISuperCluster =
          adjacent_groupby(inputSegments.begin(), inputSegments.end(), [](const CDCRecoSegment2D & segment) -> int { return segment.getISuperCluster();});

        for (const ConstVectorRange<CDCRecoSegment2D>& segmentsInSuperCluster : m_segmentsByISuperCluster) {
          std::vector<CDCRecoSegment2D> symmetricSegmentsInSuperCluster;
          symmetricSegmentsInSuperCluster.reserve(2 * segmentsInSuperCluster.size());

          for (const CDCRecoSegment2D& segment : segmentsInSuperCluster) {
            symmetricSegmentsInSuperCluster.push_back(segment);
            symmetricSegmentsInSuperCluster.push_back(segment.reversed());
          }

          WeightedNeighborhood<const CDCRecoSegment2D> segmentsNeighborhood;
          segmentsNeighborhood.clear();
          segmentsNeighborhood.createUsing(m_segmentRelationFilter,
                                           symmetricSegmentsInSuperCluster);

          MultipassCellularPathFinder<CDCRecoSegment2D> cellularPathFinder;
          std::vector< std::vector<const CDCRecoSegment2D*> > segmentPaths;
          cellularPathFinder.apply(symmetricSegmentsInSuperCluster,
                                   segmentsNeighborhood,
                                   segmentPaths);

          for (const std::vector<const CDCRecoSegment2D*>& segmentPath : segmentPaths) {
            outputSegments.push_back(CDCRecoSegment2D::condense(segmentPath));
          }
        } // end super cluster loop
      }

    private:
      /// Memory for the wire hit neighborhood in a cluster.
      WeightedNeighborhood<const CDCRecoSegment2D> m_segmentNeighborhood;

      /// Wire hit neighborhood relation filter
      SegmentRelationFilter m_segmentRelationFilter;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
