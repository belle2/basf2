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
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentLinker.h>

#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentAliasResolver.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentOrienter.h>

#include <tracking/trackFindingCDC/filters/wireHitRelation/BridgingWireHitRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/ChooseableFacetRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/minimal/WeightedRelationCreator.h>
#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Findlet implementing the segment finding part of the full track finder
    class SegmentFinderFacetAutomaton : public Findlet<CDCWireHit&, CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&, CDCSegment2D>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      SegmentFinderFacetAutomaton();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signal the beginning of a new event
      void beginEvent() final;

      /// Generates the segment from wire hits
      void apply(std::vector<CDCWireHit>& inputWireHits, std::vector<CDCSegment2D>& outputSegments) final;

    private:
      // Findlets
      /// Composes the super clusters.
      SuperClusterCreator m_superClusterCreator;

      /// Creates the clusters from super clusters
      ClusterRefiner<BridgingWireHitRelationFilter> m_clusterRefiner;

      /// Marks the clusters as background
      ClusterBackgroundDetector m_clusterBackgroundDetector;

      /// Creates the facet (hit triplet) cells of the cellular automaton
      FacetCreator m_facetCreator;

      /// Creates the facet (hit triplet) relations of the cellular automaton
      WeightedRelationCreator<const CDCFacet, ChooseableFacetRelationFilter> m_facetRelationCreator;

      /// Find the segments by composition of facets path from a cellular automaton
      SegmentCreatorFacetAutomaton m_segmentCreatorFacetAutomaton;

      /// Fits the generated segments
      SegmentFitter m_segmentFitter;

      /// Resolves ambiguous right left information alias segments and hits
      SegmentAliasResolver m_segmentAliasResolver;

      /// Adjustes the orientation of the generated segments to a prefered direction of flight
      SegmentOrienter m_segmentOrienter;

      /// Link segments with closeby segments of the same super cluster
      SegmentLinker m_segmentLinker;

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
      std::vector<CDCSegment2D> m_segments;

      /// Memory for the reconstructed segments
      std::vector<CDCSegment2D> m_intermediateSegments;
    };
  }
}
