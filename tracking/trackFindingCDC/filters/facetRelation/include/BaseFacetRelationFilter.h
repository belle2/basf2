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

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

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
      template <class AFacetIt>
      boost::iterator_range<AFacetIt>
      getPossibleNeighbors(const CDCFacet& facet, const AFacetIt& itBegin, const AFacetIt& itEnd) const
      {
        const CDCRLWireHitPair& rearRLWireHitPair = facet.getRearRLWireHitPair();
        std::pair<AFacetIt, AFacetIt> neighbors = std::equal_range(itBegin, itEnd, rearRLWireHitPair);
        return {neighbors.first, neighbors.second};
      }

      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all facet neighbors.
       */
      virtual Weight operator()(const CDCFacet& from  __attribute__((unused)),
                                const CDCFacet& to  __attribute__((unused)))
      {
        return NAN;
      }

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<const CDCFacet>& relation) override
      {
        const CDCFacet* ptrFrom(relation.getFrom());
        const CDCFacet* ptrTo(relation.getTo());
        if ((ptrFrom == nullptr) or (ptrTo == nullptr)) return NAN;
        return operator()(*ptrFrom, *ptrTo);
      }
    };
  }
}
