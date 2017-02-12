/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter accepting all facet relations with a default weight.
    class AllFacetRelationFilter : public BaseFacetRelationFilter {

    public:
      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return -2 for all valid combinations to accepting all facets,
       *  but compensating for overlap.
       */
      Weight operator()(const CDCFacet& fromFacet, const CDCFacet& toFacet) final;
    };
  }
}
