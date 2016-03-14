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
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/filters/wirehit_wirehit/WireHitNeighborChooser.h>

#include <tracking/trackFindingCDC/algorithms/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/algorithms/Clusterizer.h>

#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

#include <tracking/trackFindingCDC/creators/FacetCreator.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Worker for building reconstructed segments form wirehits using reconstructed facets
    template<class FacetFilter, class FacetNeighborChooser, bool generateSymmetric = true>
    class FacetSegmentWorker {

    public:
      /** Constructor. */
      FacetSegmentWorker(bool copyToDataStoreForDebug = true): m_copyToDataStoreForDebug(copyToDataStoreForDebug), m_cellularPathFinder(3.0) {;}

      /** Destructor.*/
      ~FacetSegmentWorker() {;}

      /// Forwards the initialize method of the module to the facet creator and the neighborhood chooser
      void initialize() {

#ifdef TRACKFINDINGCDC_USE_ROOT_BASE
        if (m_copyToDataStoreForDebug) {
          StoreArray < CDCRecoTangentSegment >::registerTransient();
          StoreArray < CDCRecoSegment2D >::registerTransient();
          StoreArray < CDCWireHitCluster >::registerTransient();
        }
#endif

        StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> >::registerTransient("CDCRecoSegment2DVector");

        m_clustersInSuperLayer.reserve(20);

        m_facetFilter.initialize();
        m_facetNeighborChooser.initialize();

      }



      /// Forwards the terminate method of the module to the facet creator and the neighborhood chooser
      void terminate() {
        m_facetFilter.terminate();
        m_facetNeighborChooser.terminate();
      }



      /// Generates the segments from the event topology.
      inline void generate(std::vector< CDCRecoSegment2D >& outputSegments) {

        m_segments2D.clear();
        outputSegments.clear();
        m_clusters.clear();
        m_recoTangentSegments.clear();

        const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
        const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

        for (const CDCWireSuperLayer & wireSuperLayer : wireTopology.getWireSuperLayers()) {

          const CDCWireHitTopology::CDCWireHitRange wireHitsInSuperlayer = wireHitTopology.getWireHits(wireSuperLayer);
          //create the neighborhood
          B2DEBUG(100, "Creating the CDCWireHit neighborhood");
          m_wirehitNeighborhood.clear();

          B2DEBUG(100, "  Append clockwise neighborhood");
          m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CW_NEIGHBOR>>(wireHitsInSuperlayer);

          B2DEBUG(100, "  Append clockwise out neighborhood");
          m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CW_OUT_NEIGHBOR>>(wireHitsInSuperlayer);

          B2DEBUG(100, "  Append clockwise in neighborhood");
          m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CW_IN_NEIGHBOR>>(wireHitsInSuperlayer);

          B2DEBUG(100, "  Append counter clockwise neighborhood");
          m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CCW_NEIGHBOR>>(wireHitsInSuperlayer);

          B2DEBUG(100, "  Append counter clockwise out neighborhood");
          m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CCW_OUT_NEIGHBOR>>(wireHitsInSuperlayer);

          B2DEBUG(100, "  Append counter clockwise in neighborhood");
          m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CCW_IN_NEIGHBOR>>(wireHitsInSuperlayer);


          bool isSymmetric = m_wirehitNeighborhood.isSymmetric();
          B2DEBUG(100, "  Check symmetry " << isSymmetric);
          B2DEBUG(100, "  allWirehitNeighbors.size() = " << m_wirehitNeighborhood.size());
          if (not isSymmetric) { B2WARNING("  The wire neighborhood is not symmetric"); }

          //create the clusters
          B2DEBUG(100, "Creating the CDCWireHit clusters");
          m_clustersInSuperLayer.clear();
          m_wirehitClusterizer.create(wireHitsInSuperlayer, m_wirehitNeighborhood, m_clustersInSuperLayer);
          B2DEBUG(100, "Created " << m_clustersInSuperLayer.size() << " CDCWireHit clusters in superlayer");

          if (m_copyToDataStoreForDebug) {
            m_clusters.insert(m_clusters.end(), m_clustersInSuperLayer.begin(), m_clustersInSuperLayer.end());
          }

          for (CDCWireHitCluster & cluster : m_clustersInSuperLayer) {
            //size_t nSegmentsBefore = m_segments2D.size();

            B2DEBUG(100, "Cluster size: " << cluster.size());
            B2DEBUG(100, "Wire hit neighborhood size: " << m_wirehitNeighborhood.size());

            //create the facets
            B2DEBUG(100, "Creating the CDCRecoFacets");
            m_facets.clear();
            m_facetCreator.createFacets(m_facetFilter, cluster, m_wirehitNeighborhood, m_facets);
            B2DEBUG(100, "  Created " << m_facets.size()  << " CDCRecoFacets");

            //create the facet neighborhood
            B2DEBUG(100, "Creating the CDCRecoFacet neighborhood");
            m_facetsNeighborhood.clear();
            m_facetsNeighborhood.createUsing(m_facetNeighborChooser, m_facets);
            B2DEBUG(100, "  Created " << m_facetsNeighborhood.size()  << " FacetsNeighborhoods");

            //Apply the cellular automaton in a multipass manner
            m_facetPaths.clear();
            m_cellularPathFinder.apply(m_facets, m_facetsNeighborhood, m_facetPaths);

            if (m_copyToDataStoreForDebug) {
              //save the tangents for display only
              for (std::vector<const CDCRecoFacet*> facetPath : m_facetPaths) {
                m_recoTangentSegments.push_back(CDCRecoTangentSegment::condense(facetPath));
              }
            }

            // reduce the CDCRecoFacetPtrSegment directly to the selected vector
            B2DEBUG(100, "Reduce the CDCRecoFacetPtrSegment to RecoSegment2D");
            if (generateSymmetric) {
              //Make enough space for the segments condensed from the facet paths and their reversed versions.
              m_segments2D.reserve(m_segments2D.size() + 2 * m_facetPaths.size());
              for (const std::vector<const CDCRecoFacet* >& facetPath : m_facetPaths) {
                m_segments2D.push_back(CDCRecoSegment2D::condense(facetPath));
                m_segments2D.push_back(m_segments2D.back().reversed());
                // ^ Save because we reserved the memory beforehand.
              }
            } else {
              // Only keep on hypotheses namely the on that is moving to the outside of the detector
              m_segments2D.reserve(m_segments2D.size() + m_facetPaths.size());
              for (const std::vector<const CDCRecoFacet* >& facetPath : m_facetPaths) {
                m_segments2D.push_back(CDCRecoSegment2D::condense(facetPath));
                // Check if the segment needs to be reversed
                CDCRecoSegment2D& lastSegment = m_segments2D.back();
                const CDCRecoHit2D& firstHit = lastSegment.front();
                const CDCRecoHit2D& lastHit = lastSegment.back();
                if (lastHit.getRecoPos2D().polarR() > firstHit.getRecoPos2D().polarR()) {
                  lastSegment.reverse();
                }
              }
            }
            //TODO: combine matching segments here

            //size_t nSegmentsAfter = m_segments2D.size();
            B2DEBUG(100, "  Created " << m_segments2D.size()  << " selected CDCRecoSegment2Ds");
          } // end for cluster


          //TODO: or combine matching segments here

        } // end for superlayer

        if (m_copyToDataStoreForDebug) {
          copyToDataStoreForDebug();
        }


        StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > wrappedStoredSegments2D("CDCRecoSegment2DVector");
        wrappedStoredSegments2D.create();
        std::vector<CDCRecoSegment2D>& storedSegments2D =  *wrappedStoredSegments2D;
        for (const CDCRecoSegment2D & segment2D : m_segments2D) {
          storedSegments2D.push_back(segment2D);
        }

        outputSegments.swap(m_segments2D);
      }

    private:
      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const {


        // IO for monitoring in python
#ifdef TRACKFINDINGCDC_USE_ROOT_BASE
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
      /// Switch to write out the intermediate data objects out to DataStore.
      bool m_copyToDataStoreForDebug;

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

      /// Memory for the tangent segments extracted from the paths
      std::vector< CDCRecoTangentSegment > m_recoTangentSegments;

      /// Memory for the hit clusters
      std::vector<CDCWireHitCluster> m_clusters;

      /// Memory for the segments extracted from the paths
      std::vector<CDCRecoSegment2D> m_segments2D;

      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Memory for the hit clusters in the current superlayer
      std::vector<CDCWireHitCluster> m_clustersInSuperLayer;

      //object creators

      //Instance of the facet filter
      FacetFilter m_facetFilter;

      /// Instance of the facet creator
      FacetCreator m_facetCreator;

      /// Instance of the filter used in edge / neighborhood creation.
      FacetNeighborChooser m_facetNeighborChooser;

      //cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<CDCRecoFacet> m_cellularPathFinder;

    }; // end class FacetSegmentWorker
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //FACETSEGMENTWORKER_H_
