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

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <vector>
#include <set>
#include <algorithm>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Class providing construction combinatorics for the facets.
    class FacetCreator {

    public:
      /// The neighborhood type used to generate wire hit triples
      typedef WeightedNeighborhood<const CDCWireHit> Neighborhood;

      template<class FacetFilter, class CDCWireHitMayBePtrRange>
      void createFacets(FacetFilter& facetFilter,
                        const CDCWireHitMayBePtrRange& wirehits,
                        const Neighborhood& neighborhood,
                        std::vector<CDCFacet>& facets) const
      {
        createFacetsGeneric(facetFilter, wirehits, neighborhood, facets);
        std::sort(facets.begin(), facets.end());
      }

    private:
      /** Generates facets on the given wire hits generating neighboring triples of hits.
       *  Inserts the result to the end of the GenericFacetCollection.
       */
      template<class FacetFilter, class CDCWireHitMayBePtrRange>
      void createFacetsGeneric(FacetFilter& facetFilter,
                               const CDCWireHitMayBePtrRange& wirehits,
                               const Neighborhood& neighborhood,
                               std::vector<CDCFacet>& facets) const
      {

        for (const auto& mayBePtrMiddleWireHit : wirehits) {

          Neighborhood::range nextNeighborRange = neighborhood.equal_range(mayBePtrMiddleWireHit);

          if (nextNeighborRange.first != nextNeighborRange.second) {

            const CDCWireHit* ptrMiddleWireHit = nextNeighborRange.first.getItem();
            if (not ptrMiddleWireHit) continue;
            const CDCWireHit& middleWireHit = *ptrMiddleWireHit;
            if (middleWireHit.getAutomatonCell().hasTakenFlag()) continue;

            for (Neighborhood::iterator itStartWireHit = nextNeighborRange.first;
                 itStartWireHit != nextNeighborRange.second; ++itStartWireHit) {

              const CDCWireHit* ptrStartWireHit = itStartWireHit.getNeighbor();
              if (not ptrStartWireHit) continue;
              const CDCWireHit& startWireHit = *ptrStartWireHit;
              if (startWireHit.getAutomatonCell().hasTakenFlag()) continue;


              for (Neighborhood::iterator itEndWireHit = nextNeighborRange.first;
                   itEndWireHit != nextNeighborRange.second; ++itEndWireHit) {

                const CDCWireHit* ptrEndWireHit = itEndWireHit.getNeighbor();
                if (not ptrEndWireHit) continue;
                const CDCWireHit& endWireHit = *ptrEndWireHit;
                if (endWireHit.getAutomatonCell().hasTakenFlag()) continue;

                //skip combinations where the facet starts and ends on the same wire
                if (not(ptrStartWireHit->getWire() ==  ptrEndWireHit->getWire())) {

                  createFacetsForHitTriple(facetFilter,
                                           startWireHit,
                                           middleWireHit,
                                           endWireHit,
                                           facets);
                }
              } //end for itEndWireHit
            } //end for itStartWireHit
          } //end if neighborRange.first != neighborRange.second
        } //end for itMiddleWireHit
      }

      /** Generates reconstruted facets on the three given wire hits by hypothesizing
       *  over the 8 left right passage combinations.
       *  Inserts the result to the end of the GenericFacetCollection.
       */
      template<class FacetFilter>
      void createFacetsForHitTriple(FacetFilter& facetFilter,
                                    const CDCWireHit& startWireHit,
                                    const CDCWireHit& middleWireHit,
                                    const CDCWireHit& endWireHit,
                                    std::vector<CDCFacet>& facets) const
      {

        const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

        auto startRLWireHits = wireHitTopology.getRLWireHits(startWireHit);
        auto middleRLWireHits = wireHitTopology.getRLWireHits(middleWireHit);
        auto endRLWireHits = wireHitTopology.getRLWireHits(endWireHit);

        for (const CDCRLWireHit& startRLWireHit : startRLWireHits) {
          for (const CDCRLWireHit& middleRLWireHit : middleRLWireHits) {
            for (const CDCRLWireHit& endRLWireHit : endRLWireHits) {

              CDCFacet facet(&startRLWireHit, &middleRLWireHit, &endRLWireHit, ParameterLine2D());
              // do not set the lines yet. The filter shall do that if it wants to.
              // He should set them if he accepts the facet.

              // Obtain a constant interface to pass to the filter method following
              const CDCFacet& constFacet = facet;

              CellWeight cellWeight = facetFilter(constFacet);

              if (not isNotACell(cellWeight)) {
                facet.getAutomatonCell().setCellWeight(cellWeight);
                facets.insert(facets.end(), facet);
              }
            } //end for endRLWireHit
          } //end for middleRLWireHit
        } //end for startRLWireHit
      }

    }; //end class FacetCreator
  } //end namespace TrackFindingCDC
} //end namespace Belle2
