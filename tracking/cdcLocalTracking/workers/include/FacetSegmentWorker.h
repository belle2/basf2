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

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>
#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>
#include <tracking/cdcLocalTracking/algorithms/NeighborhoodBuilder.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/WireHitNeighborChooser.h>

#include <tracking/cdcLocalTracking/algorithms/CellularAutomaton.h>
#include <tracking/cdcLocalTracking/algorithms/CellularPathFollower.h>
#include <tracking/cdcLocalTracking/algorithms/MultipassCellularPathFinder.h>
#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>

#include <tracking/cdcLocalTracking/creators/FacetCreator.h>
#include <tracking/cdcLocalTracking/creators/RecoSegmentCreator.h>
#include <tracking/cdcLocalTracking/creators/TangentSegmentCreator.h>

#include <tracking/cdcLocalTracking/creators/SegmentSelecter.h>
#include <tracking/cdcLocalTracking/creators/SegmentReverser.h>

#include <tracking/cdcLocalTracking/mclookup/CDCMCLookUp.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    // Worker for building reconstructed segments form wirehits using reconstructed facets
    template<class FacetFilter, class FacetNeighborChooser>
    class FacetSegmentWorker {

    public:
      //switches
      static const bool growMany = false;


      /** Constructor. */
      FacetSegmentWorker(): m_cellularPathFinder(3.0) {;}

      /** Destructor.*/
      ~FacetSegmentWorker() {;}

      void init() {

#ifdef CDCLOCALTRACKING_USE_ROOT
        StoreArray < CDCRecoTangentCollection >::registerTransient("CDCRecoTangentSegments");
        StoreArray < CDCRecoSegment2D >::registerTransient("CDCRecoHit2DSegments");
        StoreArray < CDCRecoSegment2D >::registerTransient("CDCRecoHit2DSegmentsSelected");
        StoreArray < CDCWireHitCluster >::registerTransient("CDCWireHitClusters");
#endif

        m_clusters.reserve(50);

      }

      inline void apply(const CDCWireHitCollection& wirehits,
                        std::vector< CDCRecoSegment2D >& outputSegments) {

        //which recosegment should be show in the output

        std::vector< CDCRecoSegment2D >& recoSegments = m_recoSegments;
        std::vector< CDCRecoSegment2D >& selectedRecoSegments = outputSegments;  //output

        //std::vector< CDCRecoSegment2D > & recoSegments = recoSegments; //output
        //std::vector< CDCRecoSegment2D > & selectedRecoSegments = m_recoSegments;

        recoSegments.clear();
        selectedRecoSegments.clear();

#ifdef CDCLOCALTRACKING_USE_ROOT
        m_recoTangentSegments.clear();
#endif

        //create the neighborhood
        B2DEBUG(100, "Creating the CDCWireHit neighborhood");
        m_wirehitNeighborhood.clear();
        B2DEBUG(100, "  Append clockwise neighborhood");
        m_clockwise_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append clockwise out neighborhood");
        m_clockwiseOut_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append clockwise in neighborhood");
        m_clockwiseIn_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append counter clockwise neighborhood");
        m_counterClockwise_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append counter clockwise out neighborhood");
        m_counterClockwiseOut_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append counter clockwise in neighborhood");
        m_counterClockwiseIn_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);

        bool isSymmetric = m_wirehitNeighborhood.isSymmetric();
        B2DEBUG(100, "  Check symmetry " << isSymmetric);
        B2DEBUG(100, "  allWirehitNeighbors.size() = " << m_wirehitNeighborhood.size());
        if (not isSymmetric) { B2WARNING("  The wire neighborhood is not symmetric"); }

        //create the clusters
        B2DEBUG(100, "Creating the CDCWireHit clusters");
        m_clusters.clear();
        m_wirehitClusterizer.create(wirehits, m_wirehitNeighborhood, m_clusters);

        B2DEBUG(100, "Created " << m_clusters.size() << " CDCWireHit clusters");

        //double d;
        //std::cin >> d;

        for (std::vector<CDCWireHitCluster>::iterator itCluster = m_clusters.begin();
             itCluster != m_clusters.end(); ++itCluster) {
          CDCWireHitCluster& cluster = *itCluster;


          //size_t nSegmentsBefore = selectedRecoSegments.size();

          //create the facets
          B2DEBUG(100, "Creating the CDCRecoFacets");
          m_facets.clear();
          m_facetCreator.createFacets(cluster, m_wirehitNeighborhood , m_facets);
          B2DEBUG(100, "  Created " << m_facets.size()  << " CDCRecoFacets");

          //create the facet neighborhood
          B2DEBUG(100, "Creating the CDCRecoFacet neighborhood");
          m_facetsNeighborhood.clear();
          m_facetNeighborhoodBuilder.create(m_facets, m_facetsNeighborhood);
          B2DEBUG(100, "  Created " << m_facetsNeighborhood.size()  << " FacetsNeighborhoods");


          if (growMany) {
            // single pass of cellular automation
            // all maximal segments are grown but
            // they can be very much overlapping
            // a best candidate analysis is needed
            // to single out the good ones from the many segments

            //apply the cellular automation
            B2DEBUG(100, "Apply cellular automat");
            const CDCRecoFacet* highestCell = m_cellularAutomaton.applyTo(m_facets, m_facetsNeighborhood);
            if (highestCell != nullptr) {
              B2DEBUG(100, "  MaximalState " << highestCell->getAutomatonCell().getCellState());
            }

            //create the segments by following the highest states in the reco facets
            B2DEBUG(100, "Follow the longest paths");
            m_facetSegments.clear();
            m_cellularFollower.followAll(m_facets, m_facetsNeighborhood ,
                                         m_facetSegments, 2.0);

            B2DEBUG(100, "  Created " << m_facetSegments.size()  << " CDCRecoFacetPtrSegment");

            //save the tangents for display only
#ifdef CDCLOCALTRACKING_USE_ROOT
            B2DEBUG(100, "Reduce the CDCRecoFacetPtrSegment to RecoSegment2D");
            m_tangentSegmentCreator.create(m_facetSegments, m_recoTangentSegments);
#endif

            //reduce for further usage in the best candidate analysis
            B2DEBUG(100, "Reduce the CDCRecoFacetPtrSegment to RecoSegment2D");
            // reduce the many segments
            recoSegments.clear();
            recoSegments.reserve(m_facetSegments.size());
            m_recoSegmentCreator.create(m_facetSegments, recoSegments);
            B2DEBUG(100, "  Created " << recoSegments.size()  << " CDCRecoSegment2Ds");


            //make the die out analysis to single out the best ones
            B2DEBUG(100, "Selecting CDCRecoSegment2Ds ");
            m_segmentSelecter.selectSegments(recoSegments, selectedRecoSegments);

          } else { /* not growMany */

            m_facetSegments.clear();
            m_cellularPathFinder.apply(m_facets, m_facetsNeighborhood, m_facetSegments);


            //save the tangents for display only
#ifdef CDCLOCALTRACKING_USE_ROOT
            B2DEBUG(100, "Reduce the CDCRecoFacetPtrSegment to RecoSegment2D");
            m_tangentSegmentCreator.create(m_facetSegments, m_recoTangentSegments);
#endif

            // reduce the CDCRecoFacetPtrSegment directly to the selected vector
            B2DEBUG(100, "Reduce the CDCRecoFacetPtrSegment to RecoSegment2D");
            selectedRecoSegments.reserve(selectedRecoSegments.size() + m_facetSegments.size());
            m_recoSegmentCreator.create(m_facetSegments, selectedRecoSegments);


          } //end if growMany

          //TODO: combine matching segments here

          //size_t nSegmentsAfter = selectedRecoSegments.size();

          B2DEBUG(100, "  Created " << selectedRecoSegments.size()  << " selected CDCRecoSegment2Ds");
        } // end cluster batch loop


        //make both orientations available
        B2DEBUG(100, "Reversing CDCReco2DSegments");
        m_segmentReverser.appendReversed(selectedRecoSegments);
        B2DEBUG(100, "  Created " << selectedRecoSegments.size()  << " selected CDCRecoSegment2Ds after reversion");

        //TODO: combine matching segments or here

        // IO for monitoring in python
#ifdef CDCLOCALTRACKING_USE_ROOT
        // IO wire hit clusters

        B2DEBUG(100, "  Creating the StoreArray for the CDCWireHitClusters");
        StoreArray < CDCWireHitCluster > storedClusters("CDCWireHitClusters");
        storedClusters.create();
        B2DEBUG(100, "  Do creating the CDCWireHitCluster in the StoreArray");
        BOOST_FOREACH(const CDCWireHitCluster & cluster, m_clusters) {
          storedClusters.appendNew(cluster);
        }
        B2DEBUG(100, "  Created " << storedClusters.getEntries()  << " CDCWireHitClusters");

        // IO segments with tangents
        B2DEBUG(100, "  Creating the StoreArray for the CDCRecoTangentSegment");
        StoreArray < CDCRecoTangentCollection > storedTangentSegments("CDCRecoTangentSegments");
        storedTangentSegments.create();
        B2DEBUG(100, "  Do creating the CDCRecoTangentSegment in the StoreArray");
        BOOST_FOREACH(const CDCRecoTangentCollection & tangentSegment, m_recoTangentSegments) {
          storedTangentSegments.appendNew(tangentSegment);
        }
        B2DEBUG(100, "  Created " << storedTangentSegments.getEntries()  << " CDCRecoTangentSegment");


        // IO segments without tangents

        B2DEBUG(100, "  Creating the StoreArray for the CDCRecoHit2DSegments");
        StoreArray < CDCRecoSegment2D > storedRecoSegments("CDCRecoHit2DSegments");
        storedRecoSegments.create();
        B2DEBUG(100, "  Do creating the CDCRecoSegment2D in the StoreArray");
        BOOST_FOREACH(const CDCRecoSegment2D & segment, recoSegments) {
          storedRecoSegments.appendNew(segment);
        }
        B2DEBUG(100, "  Created " << storedRecoSegments.getEntries()  << " CDCRecoSegment2D");


        // IO selected segments without tangents
        B2DEBUG(100, "  Creating the StoreArray for the selected CDCRecoHit2DSegments");
        StoreArray < CDCRecoSegment2D > storedSelectedRecoSegments("CDCRecoHit2DSegmentsSelected");
        storedSelectedRecoSegments.create();
        B2DEBUG(100, "  Copying the selected CDCRecoHit2DSegments to the StoreArray");
        BOOST_FOREACH(const CDCRecoSegment2D & segment, selectedRecoSegments) {
          storedSelectedRecoSegments.appendNew(segment);
        }
        B2DEBUG(100, "  Created " << storedSelectedRecoSegments.getEntries()  <<
                " selected CDCRecoSegment2D");
#endif
      }

    private:
      //typedefs

    private:
      //object pools
      typedef WeightedNeighborhood<const CDCWireHit> CDCWireHitNeighborhood;
      CDCWireHitNeighborhood m_wirehitNeighborhood;

      CDCRecoFacetCollection m_facets;

      typedef WeightedNeighborhood<const CDCRecoFacet> CDCRecoFacetNeighborhood;
      CDCRecoFacetNeighborhood m_facetsNeighborhood;

      std::vector< CDCRecoFacetPtrSegment > m_facetSegments;

#ifdef CDCLOCALTRACKING_USE_ROOT
      std::vector< CDCRecoTangentCollection > m_recoTangentSegments;
#endif

      std::vector<CDCRecoSegment2D> m_recoSegments;

      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      std::vector<CDCWireHitCluster> m_clusters;

      //object creators
      FacetCreator<FacetFilter> m_facetCreator;

      //neighborhood builders
      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CW_OUT_NEIGHBOR> >
      m_clockwiseOut_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CW_NEIGHBOR> >
      m_clockwise_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CW_IN_NEIGHBOR> >
      m_clockwiseIn_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CCW_OUT_NEIGHBOR> >
      m_counterClockwiseOut_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CCW_NEIGHBOR> >
      m_counterClockwise_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CCW_IN_NEIGHBOR> >
      m_counterClockwiseIn_neighborhoodBuilder;

      NeighborhoodBuilder<CDCRecoFacet, FacetNeighborChooser>
      m_facetNeighborhoodBuilder;


      //cellular automat
      CellularAutomaton<CDCRecoFacet>  m_cellularAutomaton;

      //CellularPathFollower< CDCRecoFacetCollection > m_cellularFollower;
      CellularPathFollower<CDCRecoFacet> m_cellularFollower;

      MultipassCellularPathFinder<CDCRecoFacet> m_cellularPathFinder;


      RecoSegmentCreator m_recoSegmentCreator;
      TangentSegmentCreator m_tangentSegmentCreator;

      // die out analysis
      SegmentSelecter m_segmentSelecter;
      SegmentReverser m_segmentReverser;

    }; // end class FacetSegmentWorker
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //FACETSEGMENTWORKER_H_
