/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitPair.h>

#include <tracking/trackFindingCDC/ca/NeighborWeight.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Base class for filtering the neighborhood of facets.
    /** Base implementation providing the getLowestPossibleNeighbor and
     *  isStillPossibleNeighbor method using the geometry of the facet.
     *  Besides that it accepts all facets.
     */
    template<>
    class Filter<Relation<CDCFacet>>  :
                                    public FilterBase<Relation<CDCFacet>> {

    public:
      /** Returns a two iterator range covering the range of possible neighboring
       *  facets of the given facet out of the sorted range given by the two other argumets.*/
      template<class ACDCFacetIterator>
      boost::iterator_range<ACDCFacetIterator>
      getPossibleNeighbors(const CDCFacet& facet,
                           const ACDCFacetIterator& itBegin,
                           const ACDCFacetIterator& itEnd) const
      {

        const CDCRLWireHitPair& rearRLWireHitPair = facet.getRearRLWireHitPair();

        std::pair<ACDCFacetIterator, ACDCFacetIterator> rangePossibleNeighbors =
          std::equal_range(itBegin, itEnd, rearRLWireHitPair);
        return boost::iterator_range<ACDCFacetIterator>(rangePossibleNeighbors.first,
                                                        rangePossibleNeighbors.second);
      }

      /** Legacy method */
      NeighborWeight isGoodNeighbor(const CDCFacet& from ,
                                    const CDCFacet& to)
      {
        return operator()(from, to);
      }

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NOT_A_NEIGHBOR to reject all facet neighbors.
       */
      virtual NeighborWeight operator()(const CDCFacet& /* from */,
                                        const CDCFacet& /* to */)
      {
        return NOT_A_NEIGHBOR;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      virtual CellWeight operator()(const Relation<CDCFacet>& relation) override
      {
        const CDCFacet* ptrFrom = relation.first;
        const CDCFacet* ptrTo = relation.second;
        if (not ptrFrom or not ptrTo) return NOT_A_NEIGHBOR;
        return operator()(*ptrFrom, *ptrTo);
      }


    }; // end class

    /// Alias for the base class for filtering the neighborhood of facets.
    typedef Filter<Relation<CDCFacet>> BaseFacetRelationFilter;

  } //end namespace TrackFindingCDC
} //end namespace Belle2
