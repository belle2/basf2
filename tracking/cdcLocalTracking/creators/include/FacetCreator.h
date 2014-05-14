/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FACETCREATOR_H_
#define FACETCREATOR_H_

#include <vector>
#include <set>

#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>
#include <tracking/cdcLocalTracking/algorithms/SortableVector.h>
#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing construction combinatorics for the facets.
    template<class FacetFilter>
    class FacetCreator {

    public:
      FacetCreator() : m_facetFilter() {;}
      FacetCreator(const FacetFilter& facetFilter) : m_facetFilter(facetFilter) {;}
      ~FacetCreator() {;}

      //in types
      typedef WeightedNeighborhood<const CDCWireHit> Neighborhood;

      template<class CDCWireHitMayBePtrRange>
      void createFacets(const CDCWireHitMayBePtrRange& wirehits,
                        const Neighborhood& neighborhood,
                        SortableVector<CDCRecoFacet>& facets) const {

        createFacetsGeneric(wirehits, neighborhood, facets);
        facets.sort();

      }

      template<class CDCWireHitMayBePtrRange>
      void createFacets(const CDCWireHitMayBePtrRange& wirehits,
                        const Neighborhood& neighborhood,
                        std::set<CDCRecoFacet>& facets) const {
        createFacetsGeneric(wirehits, neighborhood, facets);
      }

      template<class CDCWireHitMayBePtrRange>
      void createFacets(const CDCWireHitMayBePtrRange& wirehits,
                        const Neighborhood& neighborhood,
                        std::vector<CDCRecoFacet>& facets) const {
        createFacetsGeneric(wirehits, neighborhood, facets);
        std::sort(facets.begin(), facets.end());
      }



      void initialize() {
        m_facetFilter.initialize();
      }



      void terminate() {
        m_facetFilter.terminate();
      }


    private:

      template<class CDCWireHitMayBePtrRange, class GenericFacetCollection>
      void createFacetsGeneric(const CDCWireHitMayBePtrRange& wirehits,
                               const Neighborhood& neighborhood,
                               GenericFacetCollection& facets) const {
        m_facetFilter.clear();
        for (const auto & mayBePtrMiddleWireHit : wirehits) {

          Neighborhood::range nextNeighborRange = neighborhood.equal_range(mayBePtrMiddleWireHit);

          if (nextNeighborRange.first != nextNeighborRange.second) {

            const CDCWireHit* ptrMiddleWireHit = nextNeighborRange.first.getItem();

            for (Neighborhood::iterator itStartWireHit = nextNeighborRange.first;
                 itStartWireHit != nextNeighborRange.second; ++itStartWireHit) {

              const CDCWireHit* ptrStartWireHit = itStartWireHit.getNeighbor();

              for (Neighborhood::iterator itEndWireHit = nextNeighborRange.first;
                   itEndWireHit != nextNeighborRange.second; ++itEndWireHit) {

                const CDCWireHit* ptrEndWireHit = itEndWireHit.getNeighbor();

                //skip combinations where the facet starts and ends on the same wire
                if (not(ptrStartWireHit->getWire() ==  ptrEndWireHit->getWire())) {

                  createFacetsForHitTriple(ptrStartWireHit, ptrMiddleWireHit, ptrEndWireHit, facets);
                  //++nGroupsOfThree;
                }

              } //end for itEndWireHit
            } //end for itStartWireHit
          } //end if neighborRange.first != neighborRange.second
        } //end for itMiddleWireHit
        //B2DEBUG(200,"#GroupsOfThree " << nGroupsOfThree);
      }

      template<class GenericFacetCollection>
      void createFacetsForHitTriple(const CDCWireHit* startWireHit,
                                    const CDCWireHit* middleWireHit,
                                    const CDCWireHit* endWireHit,
                                    GenericFacetCollection& facets) const {

        if (startWireHit == nullptr or middleWireHit == nullptr or endWireHit == nullptr) return;

        const CDCWireHitTopology& cdcWireHitTopology = CDCWireHitTopology::getInstance();

        CDCWireHitTopology::CDCRLWireHitRange startRLWireHits = cdcWireHitTopology.getRLWireHits(*startWireHit);
        CDCWireHitTopology::CDCRLWireHitRange middleRLWireHits = cdcWireHitTopology.getRLWireHits(*middleWireHit);
        CDCWireHitTopology::CDCRLWireHitRange endRLWireHits = cdcWireHitTopology.getRLWireHits(*endWireHit);

        for (const CDCRLWireHit & startRLWireHit : startRLWireHits) {
          for (const CDCRLWireHit & middleRLWireHit : middleRLWireHits) {
            for (const CDCRLWireHit & endRLWireHit : endRLWireHits) {

              CDCRecoFacet facet(&startRLWireHit, &middleRLWireHit, &endRLWireHit, ParameterLine2D());
              // do not set the lines yet. The filter shall do that if he wants to.
              // He should set them if he accepts the facet.

              //Obtain a constant interface to pass to the filter method following
              const CDCRecoFacet& constFacet = facet;

              CellState cellWeight = m_facetFilter.isGoodFacet(constFacet);

              if (not isNotACell(cellWeight)) {
                facet.getAutomatonCell().setCellWeight(cellWeight);
                facets.insert(facets.end(), facet);
              }
            } //end for endRLWireHit
          } //end for middleRLWireHit
        } //end for startRLWireHit
      }

    private:
      FacetFilter m_facetFilter;

    }; //end class FacetCreator
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //FACETCREATOR_H_
