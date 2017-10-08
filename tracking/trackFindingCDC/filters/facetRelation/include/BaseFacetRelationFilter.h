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

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <algorithm>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    // Guard to prevent repeated instantiations
    extern template class Filter<Relation<const CDCFacet> >;

    /**
     *  Base class for filtering the neighborhood of facets.
     *  Base implementation providing the getLowestPossibleNeighbor and
     *  isStillPossibleNeighbor method using the geometry of the facet.
     *  Besides that it accepts all facets.
     */
    class BaseFacetRelationFilter : public Filter<Relation<const CDCFacet> > {

    public:
      /**
       *  Returns the selection of facets covering the range of possible neighboring
       *  facets of the given facet out of the sorted range given by the two other argumets.
       */
      std::vector<const CDCFacet*> getPossibleNeighbors(
        const CDCFacet* facet,
        const std::vector<const CDCFacet*>::const_iterator& itBegin,
        const std::vector<const CDCFacet*>::const_iterator& itEnd) const
      {
        // Expensive assert - but true one the less
        // assert(std::is_sorted(itBegin, itEnd, LessOf<Deref>()) && "Expected facets to be sorted");

        const CDCRLWireHitPair& rearRLWireHitPair = facet->getRearRLWireHitPair();
        ConstVectorRange<const CDCFacet*> neighbors{
          std::equal_range(itBegin, itEnd, &rearRLWireHitPair, LessOf<Deref>())};
        return {neighbors.begin(), neighbors.end()};
      }

      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all facet neighbors.
       */
      virtual Weight operator()(const CDCFacet& from  __attribute__((unused)),
                                const CDCFacet& to  __attribute__((unused)))
      {
        return 1;
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
        return this->operator()(*ptrFrom, *ptrTo);
      }
    };
  }
}
