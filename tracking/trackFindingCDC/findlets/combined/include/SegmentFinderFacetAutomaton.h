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

#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterRefiner.h>
#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/WeightedRelationCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentMerger.h>

#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentAliasResolver.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentExporter.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Legacy : Findlet implementing the segment finding part of the full track finder
    template<class AClusterFilter,
             class AFacetFilter,
             class AFacetRelationFilter,
             class ASegmentRelationFilter>
    class SegmentFinderFacetAutomaton :
      public Findlet<CDCWireHit&, CDCRecoSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&, CDCRecoSegment2D>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      SegmentFinderFacetAutomaton()
      {
        addProcessingSignalListener(&m_superClusterCreator);
        addProcessingSignalListener(&m_clusterRefiner);
        addProcessingSignalListener(&m_clusterBackgroundDetector);
        addProcessingSignalListener(&m_facetCreator);
        addProcessingSignalListener(&m_facetRelationCreator);
        addProcessingSignalListener(&m_segmentCreatorFacetAutomaton);
        addProcessingSignalListener(&m_segmentMerger);

        addProcessingSignalListener(&m_segmentFitter);
        addProcessingSignalListener(&m_segmentAliasResolver);
        addProcessingSignalListener(&m_segmentOrienter);
        addProcessingSignalListener(&m_segmentExporter);

        addProcessingSignalListener(&m_superClusterSwapper);
        addProcessingSignalListener(&m_clusterSwapper);
        addProcessingSignalListener(&m_facetSwapper);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Generates segments from hits using a cellular automaton build from hit triples (facets).";
      }

      /// Expose the parameters of the cluster filter to a module
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override
      {
        m_superClusterCreator.exposeParameters(moduleParamList, prefix);
        m_clusterRefiner.exposeParameters(moduleParamList, prefix);
        m_clusterBackgroundDetector.exposeParameters(moduleParamList, prefixed(prefix, "Cluster"));
        m_facetCreator.exposeParameters(moduleParamList, prefixed(prefix, "Facet"));
        m_facetRelationCreator.exposeParameters(moduleParamList, prefixed(prefix, "FacetRelation"));
        m_segmentCreatorFacetAutomaton.exposeParameters(moduleParamList, prefix);
        m_segmentMerger.exposeParameters(moduleParamList, prefixed(prefix, "SegmentRelation"));
        // FIXME : make parameter names small

        m_segmentFitter.exposeParameters(moduleParamList, prefix);
        m_segmentAliasResolver.exposeParameters(moduleParamList, prefix);
        m_segmentOrienter.exposeParameters(moduleParamList, prefix);
        m_segmentExporter.exposeParameters(moduleParamList, prefix);

        m_superClusterSwapper.exposeParameters(moduleParamList, prefix);
        m_clusterSwapper.exposeParameters(moduleParamList, prefix);
        m_facetSwapper.exposeParameters(moduleParamList, prefix);
      }

      /// Processes the current event
      void beginEvent() override
      {
        m_superClusters.clear();
        m_clusters.clear();
        m_facets.clear();
        m_facetRelations.clear();
        m_segments.clear();
        m_intermediateSegments.clear();
        Super::beginEvent();
      }

      /// Generates the segment.
      virtual void apply(std::vector<CDCWireHit>& inputWireHits,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final
      {
        m_superClusters.reserve(100);
        m_clusters.reserve(200);
        m_facets.reserve(800);
        m_facetRelations.reserve(800);
        m_segments.reserve(200);
        m_intermediateSegments.reserve(200);
        outputSegments.reserve(outputSegments.size() + 200);

        m_superClusterCreator.apply(inputWireHits, m_superClusters);
        m_clusterRefiner.apply(m_superClusters, m_clusters);
        m_clusterBackgroundDetector.apply(m_clusters);
        m_facetCreator.apply(m_clusters, m_facets);
        m_facetRelationCreator.apply(m_facets, m_facetRelations);
        m_segmentCreatorFacetAutomaton.apply(m_facets, m_facetRelations, m_segments);
        m_segmentFitter.apply(m_segments);

        m_segmentOrienter.apply(m_segments, m_intermediateSegments);
        m_segmentFitter.apply(m_intermediateSegments);

        m_segmentAliasResolver.apply(m_intermediateSegments);

        m_segmentMerger.apply(m_intermediateSegments, outputSegments);
        m_segmentFitter.apply(outputSegments);

        m_segmentExporter.apply(outputSegments);

        // Move superclusters to the DataStore
        m_superClusterSwapper.apply(m_superClusters);
        // Move clusters to the DataStore
        m_clusterSwapper.apply(m_clusters);
        // Move facets to the DataStore
        m_facetSwapper.apply(m_facets);
      }

    private:
      // Findlets
      /// Composes the super clusters.
      SuperClusterCreator m_superClusterCreator;

      /// Creates the clusters from super clusters
      ClusterRefiner<> m_clusterRefiner;

      /// Marks the clusters as background
      ClusterBackgroundDetector<AClusterFilter> m_clusterBackgroundDetector;

      /// Creates the facet (hit triplet) cells of the cellular automaton
      FacetCreator<AFacetFilter> m_facetCreator;

      /// Creates the facet (hit triplet) relations of the cellular automaton
      WeightedRelationCreator<const CDCFacet,
                              AFacetRelationFilter> m_facetRelationCreator;

      /// Find the segments by composition of facets path from a cellular automaton
      SegmentCreatorFacetAutomaton m_segmentCreatorFacetAutomaton;

      /// Fits the generated segments
      SegmentFitter m_segmentFitter;

      /// Resolves ambiguous right left information alias segments and hits
      SegmentAliasResolver m_segmentAliasResolver;

      /// Adjustes the orientation of the generated segments to a prefered direction of flight
      SegmentOrienter m_segmentOrienter;

      /// Merges segments with closeby segments of the same super cluster
      SegmentMerger<ASegmentRelationFilter> m_segmentMerger;

      /// Writes out copies of the segments as track candidates.
      SegmentExporter m_segmentExporter;

      /// Puts the internal super clusters on the DataStore
      StoreVectorSwapper<CDCWireHitCluster> m_superClusterSwapper{
        "CDCWireHitSuperClusterVector",
        "superCluster",
        "wire hit super cluster"
      };

      /// Puts the internal clusters on the DataStore
      StoreVectorSwapper<CDCWireHitCluster> m_clusterSwapper{"CDCWireHitClusterVector"};

      /// Puts the internal clusters on the DataStore
      StoreVectorSwapper<CDCFacet> m_facetSwapper{"CDCFacetVector"};

      // Object pools
      /// Memory for the wire hit clusters
      std::vector<CDCWireHitCluster> m_clusters;

      /// Memory for the wire hit super cluster
      std::vector<CDCWireHitCluster> m_superClusters;

      /// Memory for the generated facets
      std::vector<CDCFacet> m_facets;

      /// Memory for the generated facet relations
      std::vector<WeightedRelation<const CDCFacet> > m_facetRelations;

      /// Memory for the reconstructed segments
      std::vector<CDCRecoSegment2D> m_segments;

      /// Memory for the reconstructed segments
      std::vector<CDCRecoSegment2D> m_intermediateSegments;

    }; // end class SegmentFinderFacetAutomaton

  } //end namespace TrackFindingCDC
} //end namespace Belle2
