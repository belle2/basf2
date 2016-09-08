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

#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>
#include <tracking/trackFindingCDC/filters/facet/FeasibleRLFacetFilter.h>
#include <tracking/trackFindingCDC/filters/wireHitRelation/BridgingWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <boost/range/adaptor/indirected.hpp>

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
      /// Constructor adding the filter as a subordinary processing signal listener.
      FacetCreator()
      {
        addProcessingSignalListener(&m_wireHitRelationFilter);
        addProcessingSignalListener(&m_facetFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Creates hit triplet (facets) from each cluster filtered by a acceptance criterion.";
      }

      /// Add the parameters of the filter to the module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final
      {
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
                updateDriftLength(facet);
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

      /**
       *  Reestimate the drift length of all three contained drift circles.
       *  Using the additional flight direction information the accuracy of the drift length
       *  can be increased alot helping the filters following this step
       */
      void updateDriftLength(CDCFacet& facet)
      {
        static CDC::RealisticTDCCountTranslator tdcCountTranslator;

        const UncertainParameterLine2D& line = facet.getFitLine();
        Vector2D flightDirection = line->tangential();
        Vector2D centralPos2D = line->closest(facet.getMiddleWire().getRefPos2D());
        const double alpha = centralPos2D.angleWith(flightDirection);

        const FlightTimeEstimator& flightTimeEstimator = FlightTimeEstimator::instance();
        auto doUpdate = [&](CDCRLWireHit & rlWireHit, Vector2D recoPos2D) {
          const CDCWire& wire = rlWireHit.getWire();
          const CDCHit* hit = rlWireHit.getWireHit().getHit();
          const bool rl = rlWireHit.getRLInfo() == ERightLeft::c_Right;
          const double beta = 1;
          double flightTimeEstimate = flightTimeEstimator.getFlightTime2D(recoPos2D, alpha, beta);
          double driftLength = tdcCountTranslator.getDriftLength(hit->getTDCCount(),
                                                                 wire.getWireID(),
                                                                 flightTimeEstimate,
                                                                 rl,
                                                                 wire.getRefZ(),
                                                                 alpha);
          rlWireHit.setRefDriftLength(driftLength);
        };

        doUpdate(facet.getStartRLWireHit(), facet.getStartRecoPos2D());
        doUpdate(facet.getMiddleRLWireHit(), facet.getMiddleRecoPos2D());
        doUpdate(facet.getEndRLWireHit(), facet.getEndRecoPos2D());

        // More accurate implementation
        // double startDriftLength = getDriftLengthEstimate(facet.getStartRecoHit2D());
        // facet.getStartRLWireHit().setRefDriftLength(startDriftLength);

        // double middleDriftLength = getDriftLengthEstimate(facet.getMiddleRecoHit2D());
        // facet.getMiddleRLWireHit().setRefDriftLength(middleDriftLength);

        // double endDriftLength = getDriftLengthEstimate(facet.getEndRecoHit2D());
        // facet.getEndRLWireHit().setRefDriftLength(endDriftLength);
      }

      /// Estimate the drift length of hit making use of the reconstructed flight direction of the hit.
      double getDriftLengthEstimate(const CDCRecoHit2D& recoHit2D) const
      {
        static CDC::RealisticTDCCountTranslator tdcCountTranslator;
        const FlightTimeEstimator& flightTimeEstimator = FlightTimeEstimator::instance();

        Vector2D flightDirection = recoHit2D.getFlightDirection2D();
        Vector2D recoPos = recoHit2D.getRecoPos2D();
        double alpha = recoPos.angleWith(flightDirection);
        double flightTimeEstimate = flightTimeEstimator.getFlightTime2D(recoPos, alpha);

        const CDCWire& wire = recoHit2D.getWire();
        const CDCHit* hit = recoHit2D.getWireHit().getHit();
        const bool rl = recoHit2D.getRLInfo() == ERightLeft::c_Right;

        double driftLength =
          tdcCountTranslator.getDriftLength(hit->getTDCCount(),
                                            wire.getWireID(),
                                            flightTimeEstimate,
                                            rl,
                                            wire.getRefZ(),
                                            alpha);

        return driftLength;
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

    private:
      /// Memory for the wire hit neighborhood in within a cluster.
      std::vector<WeightedRelation<const CDCWireHit> > m_wireHitRelations;

    }; // end class FacetCreator
  } // end namespace TrackFindingCDC
} // end namespace Belle2
