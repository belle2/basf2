/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FACETSEGMENTWORKER_H_
#define FACETSEGMENTWORKER_H_

#include <vector>

#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>
#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

#include <tracking/cdcLocalTracking/algorithms/NeighborhoodBuilder.h>
#include <tracking/cdcLocalTracking/filters/wirehit_wirehit/WireHitNeighborChooser.h>

#include <tracking/cdcLocalTracking/algorithms/MultipassCellularPathFinder.h>
#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>

#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

#include <tracking/cdcLocalTracking/creators/FacetCreator.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Worker for building reconstructed segments form wirehits using reconstructed facets
    template<class FacetFilter, class FacetNeighborChooser>
    class FacetSegmentWorker {

    public:

      /** Constructor. */
      FacetSegmentWorker(): m_cellularPathFinder(3.0) {;}

      /** Destructor.*/
      ~FacetSegmentWorker() {;}

      /// Forwards the initialize method of the module to the facet creator and the neighborhood builder
      void initialize() {

#ifdef CDCLOCALTRACKING_USE_ROOT
        StoreArray < CDCRecoTangentSegment >::registerTransient();
        StoreArray < CDCRecoSegment2D >::registerTransient();
        StoreArray < CDCWireHitCluster >::registerTransient();
#endif

        m_clustersInSuperLayer.reserve(20);

        m_facetCreator.initialize();
        m_facetNeighborhoodBuilder.initialize();

      }



      /// Forwards the terminate method of the module to the facet creator and the neighborhood builder
      void terminate() {
        m_facetCreator.terminate();
        m_facetNeighborhoodBuilder.terminate();
      }



      /// Generates the segments from the event topology.
      inline void generate(std::vector< CDCRecoSegment2D >& outputSegments) {

        m_segments2D.clear();
        outputSegments.clear();

#ifdef CDCLOCALTRACKING_USE_ROOT
        m_clusters.clear();
        m_recoTangentSegments.clear();
#endif

        const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
        const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

        for (const CDCWireSuperLayer & wireSuperLayer : wireTopology.getWireSuperLayers()) {

          const CDCWireHitTopology::CDCWireHitRange wireHitsInSuperlayer = wireHitTopology.getWireHits(wireSuperLayer);

          //create the neighborhood
          B2DEBUG(100, "Creating the CDCWireHit neighborhood");
          m_wirehitNeighborhood.clear();
          B2DEBUG(100, "  Append clockwise neighborhood");
          m_clockwise_neighborhoodBuilder.append(wireHitsInSuperlayer, m_wirehitNeighborhood);
          B2DEBUG(100, "  Append clockwise out neighborhood");
          m_clockwiseOut_neighborhoodBuilder.append(wireHitsInSuperlayer, m_wirehitNeighborhood);
          B2DEBUG(100, "  Append clockwise in neighborhood");
          m_clockwiseIn_neighborhoodBuilder.append(wireHitsInSuperlayer, m_wirehitNeighborhood);
          B2DEBUG(100, "  Append counter clockwise neighborhood");
          m_counterClockwise_neighborhoodBuilder.append(wireHitsInSuperlayer, m_wirehitNeighborhood);
          B2DEBUG(100, "  Append counter clockwise out neighborhood");
          m_counterClockwiseOut_neighborhoodBuilder.append(wireHitsInSuperlayer, m_wirehitNeighborhood);
          B2DEBUG(100, "  Append counter clockwise in neighborhood");
          m_counterClockwiseIn_neighborhoodBuilder.append(wireHitsInSuperlayer, m_wirehitNeighborhood);

          bool isSymmetric = m_wirehitNeighborhood.isSymmetric();
          B2DEBUG(100, "  Check symmetry " << isSymmetric);
          B2DEBUG(100, "  allWirehitNeighbors.size() = " << m_wirehitNeighborhood.size());
          if (not isSymmetric) { B2WARNING("  The wire neighborhood is not symmetric"); }

          //create the clusters
          B2DEBUG(100, "Creating the CDCWireHit clusters");
          m_clustersInSuperLayer.clear();
          m_wirehitClusterizer.create(wireHitsInSuperlayer, m_wirehitNeighborhood, m_clustersInSuperLayer);
          B2DEBUG(100, "Created " << m_clustersInSuperLayer.size() << " CDCWireHit clusters in superlayer");

#ifdef CDCLOCALTRACKING_USE_ROOT
          m_clusters.insert(m_clusters.end(), m_clustersInSuperLayer.begin(), m_clustersInSuperLayer.end());
#endif


          for (CDCWireHitCluster & cluster : m_clustersInSuperLayer) {
            //size_t nSegmentsBefore = m_segments2D.size();

            B2DEBUG(100, "Cluster size: " << cluster.size());
            B2DEBUG(100, "Wire hit neighborhood size: " << m_wirehitNeighborhood.size());

            //create the facets
            B2DEBUG(100, "Creating the CDCRecoFacets");
            m_facets.clear();
            m_facetCreator.createFacets(cluster, m_wirehitNeighborhood, m_facets);
            B2DEBUG(100, "  Created " << m_facets.size()  << " CDCRecoFacets");

            //create the facet neighborhood
            B2DEBUG(100, "Creating the CDCRecoFacet neighborhood");
            m_facetsNeighborhood.clear();
            m_facetNeighborhoodBuilder.create(m_facets, m_facetsNeighborhood);
            B2DEBUG(100, "  Created " << m_facetsNeighborhood.size()  << " FacetsNeighborhoods");

            //Apply the cellular automaton in a multipass manner
            m_facetPaths.clear();
            m_cellularPathFinder.apply(m_facets, m_facetsNeighborhood, m_facetPaths);

            //save the tangents for display only
#ifdef CDCLOCALTRACKING_USE_ROOT
            for (std::vector<const CDCRecoFacet*> facetPath : m_facetPaths) {
              m_recoTangentSegments.push_back(CDCRecoTangentSegment::condense(facetPath));
            }
#endif

            // reduce the CDCRecoFacetPtrSegment directly to the selected vector
            B2DEBUG(100, "Reduce the CDCRecoFacetPtrSegment to RecoSegment2D");
            //Make enough space for the segments condensed from the facet paths and their reversed versions.
            m_segments2D.reserve(m_segments2D.size() + 2 * m_facetPaths.size());
            for (const std::vector<const CDCRecoFacet* >& facetPath : m_facetPaths) {
              m_segments2D.push_back(CDCRecoSegment2D::condense(facetPath));
              m_segments2D.push_back(m_segments2D.back().reversed());
              // ^ Save because we reserved the memory beforehand.
            }

            //TODO: combine matching segments here

            //size_t nSegmentsAfter = m_segments2D.size();
            B2DEBUG(100, "  Created " << m_segments2D.size()  << " selected CDCRecoSegment2Ds");
          } // end for cluster


          //TODO: or combine matching segments here

        } // end for superlayer

        copyToDataStoreForDebug();

        outputSegments.swap(m_segments2D);
      }

    private:
      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const {


        // IO for monitoring in python
#ifdef CDCLOCALTRACKING_USE_ROOT
        // IO wire hit clusters

        StoreArray < CDCWireHitCluster > storedClusters;
        storedClusters.create();
        for (const CDCWireHitCluster & cluster : m_clusters) {
          storedClusters.appendNew(cluster);
        }

        // IO segments with tangents
        StoreArray < CDCRecoTangentVector > storedTangentSegments("CDCRecoTangentSegments");
        storedTangentSegments.create();
        for (const CDCRecoTangentVector & tangentSegment : m_recoTangentSegments) {
          storedTangentSegments.appendNew(tangentSegment);
        }


        // IO selected segments without tangents
        StoreArray < CDCRecoSegment2D > storedSegments2D;
        storedSegments2D.create();
        for (const CDCRecoSegment2D & segment2D : m_segments2D) {
          storedSegments2D.appendNew(segment2D);
        }
#endif

      }


    private:
      //typedefs

    private:
      //object pools
      /// Neighborhood type for wire hits
      typedef WeightedNeighborhood<const CDCWireHit> CDCWireHitNeighborhood;

      /// Memory for the wire hit neighborhood.
      CDCWireHitNeighborhood m_wirehitNeighborhood;

      /// Memory for the constructed facets.
      CDCRecoFacetCollection m_facets;

      /// Neighborhood type for facets.
      typedef WeightedNeighborhood<const CDCRecoFacet> CDCRecoFacetNeighborhood;

      /// Memory for the facet neighborhood.
      CDCRecoFacetNeighborhood m_facetsNeighborhood;

      /// Memory for the facet paths generated from the graph.
      std::vector< std::vector<const CDCRecoFacet*> > m_facetPaths;

#ifdef CDCLOCALTRACKING_USE_ROOT
      /// Memory for the tangent segments extracted from the paths
      std::vector< CDCRecoTangentSegment > m_recoTangentSegments;

      /// Memory for the hit clusters
      std::vector<CDCWireHitCluster> m_clusters;
#endif

      /// Memory for the segments extracted from the paths
      std::vector<CDCRecoSegment2D> m_segments2D;

      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Memory for the hit clusters in the current superlayer
      std::vector<CDCWireHitCluster> m_clustersInSuperLayer;

      //object creators

      /// Instance of the facet creator
      FacetCreator<FacetFilter> m_facetCreator;

      //neighborhood builders
      /// Type of the neighborhood relation builder.
      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CW_OUT_NEIGHBOR> >
      /// Instance of the neighborhood builder.
      m_clockwiseOut_neighborhoodBuilder;

      /// Type of the neighborhood relation builder.
      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CW_NEIGHBOR> >
      /// Instance of the neighborhood builder.
      m_clockwise_neighborhoodBuilder;

      /// Type of the neighborhood relation builder.
      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CW_IN_NEIGHBOR> >
      /// Instance of the neighborhood builder.
      m_clockwiseIn_neighborhoodBuilder;

      /// Type of the neighborhood relation builder.
      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CCW_OUT_NEIGHBOR> >
      /// Instance of the neighborhood builder.
      m_counterClockwiseOut_neighborhoodBuilder;

      /// Type of the neighborhood relation builder.
      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CCW_NEIGHBOR> >
      /// Instance of the neighborhood builder.
      m_counterClockwise_neighborhoodBuilder;

      /// Type of the neighborhood relation builder.
      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CCW_IN_NEIGHBOR> >
      /// Instance of the neighborhood builder.
      m_counterClockwiseIn_neighborhoodBuilder;

      /// Type of the neighborhood relation builder.
      NeighborhoodBuilder<CDCRecoFacet, FacetNeighborChooser>
      /// Instance of the neighborhood builder.
      m_facetNeighborhoodBuilder;

      //cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<CDCRecoFacet> m_cellularPathFinder;


    }; // end class FacetSegmentWorker
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //FACETSEGMENTWORKER_H_
