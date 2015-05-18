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

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHitPair.h>

#include <tracking/trackFindingCDC/algorithms/NeighborWeight.h>
#include <tracking/trackFindingCDC/algorithms/Relation.h>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>
#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Base class for filtering the neighborhood of facets.
    /** Base implementation providing the getLowestPossibleNeighbor and
     *  isStillPossibleNeighbor method using the geometry of the facet.
     *  Besides that it accepts all facets.
     */
    template<>
    class Filter<Relation<CDCRecoFacet>>  :
                                        public FilterBase<Relation<CDCRecoFacet>> {

    public:
      /** Returns a two iterator range covering the range of possible neighboring
       *  facets of the given facet out of the sorted range given by the two other argumets.*/
      template<class CDCRecoFacetIterator>
      boost::iterator_range<CDCRecoFacetIterator>
      getPossibleNeighbors(const CDCRecoFacet& recoFacet,
                           const CDCRecoFacetIterator& itBegin,
                           const CDCRecoFacetIterator& itEnd) const
      {

        const CDCRLWireHitPair& rearRLWireHitPair = recoFacet.getRearRLWireHitPair();

        std::pair<CDCRecoFacetIterator, CDCRecoFacetIterator> rangePossibleNeighbors =
          std::equal_range(itBegin, itEnd, rearRLWireHitPair);
        return boost::iterator_range<CDCRecoFacetIterator>(rangePossibleNeighbors.first,
                                                           rangePossibleNeighbors.second);
      }

      /** Legacy method */
      NeighborWeight isGoodNeighbor(const CDCRecoFacet& from ,
                                    const CDCRecoFacet& to)
      {
        return operator()(from, to);
      }

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NOT_A_NEIGHBOR to reject all facet neighbors.
       */
      virtual NeighborWeight operator()(const CDCRecoFacet& /* from */,
                                        const CDCRecoFacet& /* to */)
      {
        return NOT_A_NEIGHBOR;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      virtual CellWeight operator()(const Relation<CDCRecoFacet>& relation) override
      {
        const CDCRecoFacet* ptrFrom = relation.first;
        const CDCRecoFacet* ptrTo = relation.second;
        if (not ptrFrom or not ptrTo) return NOT_A_NEIGHBOR;
        return operator()(*ptrFrom, *ptrTo);
      }


    }; // end class

    /// Alias for the base class for filtering the neighborhood of facets.
    typedef Filter<Relation<CDCRecoFacet>> BaseFacetNeighborChooser;

  } //end namespace TrackFindingCDC
} //end namespace Belle2
