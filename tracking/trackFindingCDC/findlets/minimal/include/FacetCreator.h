/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/filters/wirehit_relation/WholeWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Class providing construction combinatorics for the facets.
    template<class AFacetFilter>
    class FacetCreator : public Findlet<const CDCWireHitCluster, CDCFacet> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHitCluster, CDCFacet>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Creates hit triplet (facets) from each cluster filtered by a acceptance criterion.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList) override final
      {
        m_facetFilter.exposeParameters(moduleParamList);
      }

      /// Initialize before event processing
      virtual void initialize() override final
      {
        Super::initialize();
        m_facetFilter.initialize();
      }

      /// Start processing the current event
      virtual void beginEvent() override final
      {
        Super::beginEvent();
        m_facetFilter.beginEvent();
      }

      /// Terminate the event processing
      virtual void terminate() override final
      {
        m_facetFilter.terminate();
        Super::terminate();
      }

      virtual void apply(const std::vector<CDCWireHitCluster>& inputClusters,
                         std::vector<CDCFacet>& facets) override
      {
        int iCluster = -1;
        for (const CDCWireHitCluster& cluster : inputClusters) {
          ++iCluster;
          // Skip clusters that have been detected as background
          if (cluster.getBackgroundFlag()) {
            continue;
          }
          B2ASSERT("Expect the clusters to be sorted",
                   std::is_sorted(std::begin(cluster), std::end(cluster)));

          // Create the neighborhood of wire hits on the cluster
          B2DEBUG(100, "Creating the CDCWireHit neighborhood");
          const bool primaryNeighborhoodOnly = false;
          using PrimaryWireHitNeighborhoodFilter = WholeWireHitRelationFilter<primaryNeighborhoodOnly>;

          m_wirehitNeighborhood.clear();
          m_wirehitNeighborhood.appendUsing<PrimaryWireHitNeighborhoodFilter>(cluster);
          B2ASSERT("Wire neighborhood is not symmetric. Check the geometry.", m_wirehitNeighborhood.isSymmetric());
          B2DEBUG(100, "  wirehitNeighborhood.size() = " << m_wirehitNeighborhood.size());

          // Create the facets
          B2DEBUG(100, "Creating the CDCFacets");
          std::size_t nBefore = facets.size();
          createFacets(cluster, m_wirehitNeighborhood, facets);
          std::size_t nAfter = facets.size();

          VectorRange<CDCFacet> facetsInCluster(facets.begin() + nBefore, facets.begin() + nAfter);
          // Sort the facets in their cluster
          std::sort(facetsInCluster.begin(), facetsInCluster.end());

          for (CDCFacet& facet : facetsInCluster) {
            facet.setICluster(iCluster);
          }
        }
      }

    private:
      /** Generates facets on the given wire hits generating neighboring triples of hits.
       *  Inserts the result to the end of the GenericFacetCollection.
       */
      void createFacets(const CDCWireHitCluster& wireHits,
                        const WeightedNeighborhood<const CDCWireHit>& neighborhood,
                        std::vector<CDCFacet>& facets)
      {
        for (const CDCWireHit* ptrMiddleWireHit : wireHits) {
          if (not ptrMiddleWireHit) continue;
          const CDCWireHit& middleWireHit = *ptrMiddleWireHit;
          if (middleWireHit.getAutomatonCell().hasTakenFlag()) continue;

          const auto neighbors = neighborhood.equal_range(ptrMiddleWireHit);
          for (const CDCWireHit* ptrStartWireHit : neighbors) {

            if (not ptrStartWireHit) continue;
            const CDCWireHit& startWireHit = *ptrStartWireHit;
            if (startWireHit.getAutomatonCell().hasTakenFlag()) continue;

            for (const CDCWireHit* ptrEndWireHit : neighbors) {
              if (not ptrEndWireHit) continue;
              const CDCWireHit& endWireHit = *ptrEndWireHit;
              if (endWireHit.getAutomatonCell().hasTakenFlag()) continue;
              // Skip combinations where the facet starts and ends on the same wire
              if (not(ptrStartWireHit->getWire() ==  ptrEndWireHit->getWire())) {
                createFacetsForHitTriple(startWireHit,
                                         middleWireHit,
                                         endWireHit,
                                         facets);
              }
            } //end for itEndWireHit
          } //end for itStartWireHit
        } //end for itMiddleWireHit
      }

      /** Generates reconstruted facets on the three given wire hits by hypothesizing
       *  over the 8 left right passage combinations.
       *  Inserts the result to the end of the GenericFacetCollection.
       */
      void createFacetsForHitTriple(const CDCWireHit& startWireHit,
                                    const CDCWireHit& middleWireHit,
                                    const CDCWireHit& endWireHit,
                                    std::vector<CDCFacet>& facets)
      {
        /// Prepare a facet - without fitted tangent lines.
        CDCRLTaggedWireHit startRLWireHit(&startWireHit, ERightLeft::c_Left);
        CDCRLTaggedWireHit middleRLWireHit(&middleWireHit, ERightLeft::c_Left);
        CDCRLTaggedWireHit endRLWireHit(&endWireHit, ERightLeft::c_Left);
        CDCFacet facet(startRLWireHit, middleRLWireHit, endRLWireHit, ParameterLine2D());

        for (ERightLeft startRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
          facet.setStartRLInfo(startRLInfo);
          for (ERightLeft middleRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
            facet.setMiddleRLInfo(middleRLInfo);
            for (ERightLeft endRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
              facet.setEndRLInfo(endRLInfo);

              // Reset the lines
              // The filter shall do the fitting of the tangent lines if it wants to.
              // He should set them if he accepts the facet.
              facet.invalidateLines();

              // Obtain a constant interface to pass to the filter method following
              const CDCFacet& constFacet = facet;

              CellWeight cellWeight = m_facetFilter(constFacet);

              if (not isNotACell(cellWeight)) {
                facet.getAutomatonCell().setCellWeight(cellWeight);
                facets.insert(facets.end(), facet);
              }
            } //end for endRLWireHit
          } //end for middleRLWireHit
        } //end for startRLWireHit
      }

    private:
      /// Memory for the wire hit neighborhood in within a cluster.
      WeightedNeighborhood<const CDCWireHit> m_wirehitNeighborhood;

    private:
      /// The filter to be used for the facet generation.
      AFacetFilter m_facetFilter;

    }; //end class FacetCreator
  } //end namespace TrackFindingCDC
} //end namespace Belle2
