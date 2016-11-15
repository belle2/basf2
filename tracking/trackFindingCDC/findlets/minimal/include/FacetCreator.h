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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/facet/FeasibleRLFacetFilter.h>
#include <tracking/trackFindingCDC/filters/wireHitRelation/BridgingWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/utils/DriftLengthEstimator.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <framework/core/ModuleParamList.h>

#include <boost/range/adaptor/indirected.hpp>

#include <vector>
#include <string>
#include <algorithm>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    /// Class providing construction combinatorics for the facets.
    template<class AFacetFilter>
    class FacetCreator : public Findlet<const CDCWireHitCluster, CDCFacet> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHitCluster, CDCFacet>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      FacetCreator()
      {
        this->addProcessingSignalListener(&m_wireHitRelationFilter);
        this->addProcessingSignalListener(&m_facetFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Creates hit triplet (facets) from each cluster filtered by a acceptance criterion.";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_wireHitRelationFilter.exposeParameters(moduleParamList, prefix);
        m_feasibleRLFacetFilter.exposeParameters(moduleParamList, prefix);
        m_facetFilter.exposeParameters(moduleParamList, prefix);

        moduleParamList->addParameter(prefixed(prefix, "updateDriftLength"),
        m_param_updateDriftLength,
        "Switch to reestimate the drift length",
        m_param_updateDriftLength);

        moduleParamList->addParameter(prefixed(prefix, "leastSquareFit"),
        m_param_leastSquareFit,
        "Switch to fit the facet with the least square method "
        "for drift length estimation",
        m_param_leastSquareFit);

      }

      /**
       *  Central function creating the hit triplets from the clusters.
       *
       *  Hit triplets are always contained within one cluster and
       *  the output hit triplets follow the order of the input clusters
       *  such that the triplets of the same cluster remain close to each other.
       */
      void apply(const std::vector<CDCWireHitCluster>& inputClusters,
                 std::vector<CDCFacet>& facets) final {
        int iCluster = -1;
        for (const CDCWireHitCluster& cluster : inputClusters)
        {
          ++iCluster;
          // Skip clusters that have been detected as background
          if (cluster.getBackgroundFlag()) {
            continue;
          }
          B2ASSERT("Expect the clusters to be sorted",
                   std::is_sorted(cluster.begin(), cluster.end()));

          // Create the neighborhood of wire hits on the cluster
          B2DEBUG(100, "Creating the CDCWireHit neighborhood");
          m_wireHitRelations.clear();
          auto wireHits = cluster | boost::adaptors::indirected;
          WeightedNeighborhood<const CDCWireHit>::appendUsing(m_wireHitRelationFilter, wireHits, m_wireHitRelations);
          WeightedNeighborhood<const CDCWireHit> wirehitNeighborhood(m_wireHitRelations);

          B2ASSERT("Wire neighborhood is not symmetric. Check the geometry.", wirehitNeighborhood.isSymmetric());
          B2DEBUG(100, "  wirehitNeighborhood.size() = " << wirehitNeighborhood.size());

          // Create the facets
          B2DEBUG(100, "Creating the CDCFacets");
          std::size_t nBefore = facets.size();
          createFacets(cluster, wirehitNeighborhood, facets);
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
      /**
       *  Generates facets on the given wire hits generating neighboring triples of hits.
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
          for (const WeightedRelation<const CDCWireHit>& startWireHitRelation : neighbors) {
            const CDCWireHit* ptrStartWireHit(startWireHitRelation.getTo());

            if (not ptrStartWireHit) continue;
            const CDCWireHit& startWireHit = *ptrStartWireHit;
            if (startWireHit.getAutomatonCell().hasTakenFlag()) continue;

            for (const WeightedRelation<const CDCWireHit>& endWireHitRelation : neighbors) {
              const CDCWireHit* ptrEndWireHit(endWireHitRelation.getTo());

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
            } // end for itEndWireHit
          } // end for itStartWireHit
        } // end for itMiddleWireHit
      }

      /**
       *  Generates reconstruted facets on the three given wire hits by hypothesizing
       *  over the 8 left right passage combinations.
       *  Inserts the result to the end of the GenericFacetCollection.
       */
      void createFacetsForHitTriple(const CDCWireHit& startWireHit,
                                    const CDCWireHit& middleWireHit,
                                    const CDCWireHit& endWireHit,
                                    std::vector<CDCFacet>& facets)
      {
        /// Prepare a facet - without fitted tangent lines.
        CDCRLWireHit startRLWireHit(&startWireHit, ERightLeft::c_Left,
                                    startWireHit->getRefDriftLength());
        CDCRLWireHit middleRLWireHit(&middleWireHit, ERightLeft::c_Left,
                                     middleWireHit->getRefDriftLength());
        CDCRLWireHit endRLWireHit(&endWireHit, ERightLeft::c_Left,
                                  endWireHit->getRefDriftLength());
        CDCFacet facet(startRLWireHit, middleRLWireHit, endRLWireHit, UncertainParameterLine2D());

        for (ERightLeft startRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
          facet.setStartRLInfo(startRLInfo);
          for (ERightLeft middleRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
            facet.setMiddleRLInfo(middleRLInfo);
            for (ERightLeft endRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
              facet.setEndRLInfo(endRLInfo);

              // Reset the lines
              // The filter shall do the fitting of the tangent lines if it wants to.
              // He should set them if he accepts the facet.
              facet.invalidateFitLine();

              if (m_param_feasibleRLOnly) {
                Weight feasibleWeight = m_feasibleRLFacetFilter(facet);
                if (std::isnan(feasibleWeight)) continue;
              }

              if (m_param_updateDriftLength) {

                // Reset drift length
                facet.getStartRLWireHit().setRefDriftLength(startWireHit.getRefDriftLength());
                facet.getMiddleRLWireHit().setRefDriftLength(middleWireHit.getRefDriftLength());
                facet.getEndRLWireHit().setRefDriftLength(endWireHit.getRefDriftLength());

                if (m_param_leastSquareFit) {
                  /*double chi2 =*/ FacetFitter::fit(facet);
                } else {
                  facet.adjustFitLine();
                }

                // Update drift length
                m_driftLengthEstimator.updateDriftLength(facet);
              }

              CellWeight cellWeight = m_facetFilter(facet);

              if (not isNotACell(cellWeight)) {
                facet.getAutomatonCell().setCellWeight(cellWeight);
                facets.insert(facets.end(), facet);
              }
            } // end for endRLWireHit
          } // end for middleRLWireHit
        } // end for startRLWireHit
      }

    private:
      /// Parameter : Switch to apply the rl feasibility cut
      bool m_param_feasibleRLOnly = true;

      /// Parameter : Switch to reestimate the drift length
      bool m_param_updateDriftLength = true;

      /// Parameter : Switch to fit the facet with least square method for the drift length update
      bool m_param_leastSquareFit = false;

    private:
      /// The filter for the hit neighborhood.
      BridgingWireHitRelationFilter m_wireHitRelationFilter;

      /// The feasibility filter for the right left passage information
      FeasibleRLFacetFilter m_feasibleRLFacetFilter;

      /// The filter to be used for the facet generation.
      AFacetFilter m_facetFilter;

      /// Instance of the drift length estimator to be used.
      DriftLengthEstimator m_driftLengthEstimator;

    private:
      /// Memory for the wire hit neighborhood in within a cluster.
      std::vector<WeightedRelation<const CDCWireHit> > m_wireHitRelations;
    };
  }
}
