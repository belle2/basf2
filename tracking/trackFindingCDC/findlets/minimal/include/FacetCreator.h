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

#include <tracking/trackFindingCDC/filters/facet/ChooseableFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/FeasibleRLFacetFilter.h>
#include <tracking/trackFindingCDC/filters/wireHitRelation/BridgingWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/fitting/FacetFitter.h>
#include <tracking/trackFindingCDC/eventdata/utils/DriftLengthEstimator.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCFacet;
    class CDCWireHitCluster;

    /// Class providing construction combinatorics for the facets.
    class FacetCreator : public Findlet<const CDCWireHitCluster, CDCFacet> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHitCluster, CDCFacet>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      FacetCreator();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /**
       *  Central function creating the hit triplets from the clusters.
       *
       *  Hit triplets are always contained within one cluster and
       *  the output hit triplets follow the order of the input clusters
       *  such that the triplets of the same cluster remain close to each other.
       */
      void apply(const std::vector<CDCWireHitCluster>& inputClusters, std::vector<CDCFacet>& facets) final;

    private:
      /**
       *  Generates facets on the given wire hits generating neighboring triples of hits.
       *  Inserts the result to the end of the GenericFacetCollection.
       */
      void createFacets(const std::vector<CDCWireHit*>& wireHits,
                        const std::vector<WeightedRelation<CDCWireHit> >& wireHitRelations,
                        std::vector<CDCFacet>& facets);

      /**
       *  Generates reconstruted facets on the three given wire hits by hypothesizing
       *  over the 8 left right passage combinations.
       *  Inserts the result to the end of the GenericFacetCollection.
       */
      void createFacetsForHitTriple(const CDCWireHit& startWireHit,
                                    const CDCWireHit& middleWireHit,
                                    const CDCWireHit& endWireHit,
                                    std::vector<CDCFacet>& facets);
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
      ChooseableFacetFilter m_facetFilter;

      /// Instance of the drift length estimator to be used.
      DriftLengthEstimator m_driftLengthEstimator;

    private:
      /// Memory for the wire hit neighborhood in within a cluster.
      std::vector<WeightedRelation<CDCWireHit> > m_wireHitRelations;
    };
  }
}
