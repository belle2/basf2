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

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Base class for filtering the neighborhood of facets.
     *  Base implementation providing the getLowestPossibleNeighbor and
     *  isStillPossibleNeighbor method using the geometry of the facet.
     *  Besides that it accepts all facets.
     */
    class BaseFacetRelationFilter : public Filter<Relation<const CDCFacet> > {

    public:
      /**
       *  Returns a two iterator range covering the range of possible neighboring
       *  facets of the given facet out of the sorted range given by the two other argumets.
       */
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

      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all facet neighbors.
       */
      virtual Weight operator()(const CDCFacet& /* from */,
                                const CDCFacet& /* to */)
      {
        return NAN;
      }

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      virtual Weight operator()(const Relation<const CDCFacet>& relation) override
      {
        const CDCFacet* ptrFrom(relation.first);
        const CDCFacet* ptrTo(relation.second);
        if (not ptrFrom or not ptrTo) return NAN;
        return operator()(*ptrFrom, *ptrTo);
      }

    };
  }
}
