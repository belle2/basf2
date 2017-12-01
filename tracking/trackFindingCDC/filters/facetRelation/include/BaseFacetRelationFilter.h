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

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    // Guard to prevent repeated instantiations
    extern template class RelationFilter<const CDCFacet>;

    /// Base class for filtering the neighborhood of facets.
    class BaseFacetRelationFilter : public RelationFilter<const CDCFacet> {

    public:
      /// Default constructor
      BaseFacetRelationFilter();

      /// Default destructor
      ~BaseFacetRelationFilter();

      /**
       *  Returns the selection of facets covering the range of possible neighboring
       *  facets of the given facet out of the sorted range given by the two other argumets.
       */
      std::vector<const CDCFacet*> getPossibleTos(
        const CDCFacet* from,
        const std::vector<const CDCFacet*>& facets) const final;
    };
  }
}
