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

#include<framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing construction combinatorics for the facets.
    template<class Filter>
    class FacetCreator {

    public:
      //typedef SimpleFacetFilter Filter;

      /** Constructor. */

      FacetCreator() : m_filter() {;}
      FacetCreator(const Filter& filter) : m_filter(filter) {;}


      ~FacetCreator() {;}

      //in types
      typedef CDCWireHitCollection   WireHitCollection;
      typedef CDCWireHitCluster      WireHitCluster;
      typedef WeightedNeighborhood<const CDCWireHit> Neighborhood;

      //out type
      typedef CDCRecoFacetCollection FacetCollection;

      //normal wirehit collections - usable with the AllWireHitCollection
      void createFacets(const WireHitCollection& wirehits,
                        const Neighborhood& neighborhood,
                        FacetCollection& facets) const {

        createFacetsGeneric(wirehits, neighborhood, facets);

        B2INFO("Before sort");
        facets.sort();
      }


      void createFacets(const WireHitCollection& wirehits,
                        const Neighborhood& neighborhood,
                        std::vector< CDCRecoFacet >& facets) const {

        createFacetsGeneric(wirehits, neighborhood, facets);
        std::sort(facets.begin(), facets.end());
      }

      void createFacets(const WireHitCollection& wirehits,
                        const Neighborhood& neighborhood,
                        std::set< CDCRecoFacet >& facets) const {
        createFacetsGeneric(wirehits, neighborhood, facets);
      }

      //cluster
      void createFacets(const WireHitCluster& wirehits,
                        const Neighborhood& neighborhood,
                        FacetCollection& facets) const {


        //B2INFO("Number of facets before " << facets.size());
        createFacetsGeneric(wirehits, neighborhood, facets);
        //B2INFO("Number of facets after " << facets.size());
        facets.sort();
        return;
        /*
        std::set<size_t> addressesInFacets;

        BOOST_FOREACH(const CDCRecoFacet & recofacet,facets){
          B2INFO( recofacet );
          addressesInFacets.insert( (size_t)(recofacet.getStartWireHit()) );
          addressesInFacets.insert( (size_t)(recofacet.getMiddleWireHit()) );
          addressesInFacets.insert( (size_t)(recofacet.getEndWireHit()) );
        }

        std::cerr << "Address of sentinal " << size_t(&(CDCWireHit::getLowest())) << std::endl;

        std::set<size_t> addressesOfWirehits;

        BOOST_FOREACH(const CDCWireHit * wirehit,wirehits){
          addressesOfWirehits.insert(size_t(wirehit));
        }

        std::cerr << "Addresses of wirehits" << std::endl;
        BOOST_FOREACH(size_t address,addressesOfWirehits){
          std::cerr << address << std::endl;
        }

        std::cerr << "Addresses in facets" << std::endl;
        BOOST_FOREACH(size_t address,addressesInFacets){
          std::cerr << address << std::endl;
          if ( not addressesOfWirehits.count(address) ){
            std::cerr << "Not in addressesOfWireHits" << std::endl;
            double d;
            std::cin >> d;
          }
        }

        B2INFO("Before sort");
        std::sort( facets.begin(),facets.end());
        B2INFO("After sort");*/

      }

      void createFacets(const WireHitCluster& wirehits,
                        const Neighborhood& neighborhood,
                        std::set< CDCRecoFacet >& facets) const {
        createFacetsGeneric(wirehits, neighborhood, facets);
      }

      void createFacets(const WireHitCluster& wirehits,
                        const Neighborhood& neighborhood,
                        std::vector< CDCRecoFacet >& facets) const {
        createFacetsGeneric(wirehits, neighborhood, facets);
        std::sort(facets.begin(), facets.end());
      }


    private:

      template<class GenericWireHitCollection, class GenericFacetCollection>
      void createFacetsGeneric(const GenericWireHitCollection& wirehits,
                               const Neighborhood& neighborhood,
                               GenericFacetCollection& facets) const {

        m_filter.clear();

        //size_t nGroupsOfThree = 0;

        for (typename GenericWireHitCollection::const_iterator itMiddleWireHit = wirehits.begin();
             itMiddleWireHit != wirehits.end() ;  ++itMiddleWireHit) {

          const typename GenericWireHitCollection::Item& middleWireHit = *itMiddleWireHit;
          const CDCWireHit* ptrMiddleWireHit = &(*middleWireHit);

          Neighborhood::range nextNeighborRange = neighborhood.equal_range(ptrMiddleWireHit);

          for (Neighborhood::iterator itStartWireHit = nextNeighborRange.first;
               itStartWireHit != nextNeighborRange.second; ++itStartWireHit) {

            const CDCWireHit* ptrStartWireHit = itStartWireHit.getNeighbor();

            for (Neighborhood::iterator itEndWireHit = nextNeighborRange.first;
                 itEndWireHit != nextNeighborRange.second; ++itEndWireHit) {

              const CDCWireHit* ptrEndWireHit = itEndWireHit.getNeighbor();

              //skip combinations where the facet starts and ends on the same wire
              if (not(ptrStartWireHit->getWire() ==  ptrEndWireHit->getWire())) {
                //B2DEBUG(200,"Building facets for");
                //B2DEBUG(200,"Start hit " <<  startWireHit->getWire());
                //B2DEBUG(200,"Middle hit " <<  middleWireHit->getWire());
                //B2DEBUG(200,"End hit " <<  endWireHit->getWire());

                createFacetsForHitTriple(ptrStartWireHit, ptrMiddleWireHit, ptrEndWireHit, facets);
                //++nGroupsOfThree;
              }
            } //end for itEndWireHit
          } //end for itStartWireHit
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

              const CDCRecoFacet& constFacet = facet;

              CellState cellWeight = m_filter.isGoodFacet(constFacet);

              if (not isNotACell(cellWeight)) {
                facet.getAutomatonCell().setCellWeight(cellWeight);

                //typename GenericFacetCollection::iterator itNewFacet =
                facets.insert(facets.end(), facet);

                //itNewFacet->getAutomatonCell().setCellWeight(cellWeight);

              }

            } //end for endRLWireHit

          } //end for middleRLWireHit
        } //end for startRLWireHit
      }

    private:
      Filter m_filter;

    }; //end class FacetCreator
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //FACETCREATOR_H_
