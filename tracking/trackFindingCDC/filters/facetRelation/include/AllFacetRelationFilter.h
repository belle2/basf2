/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
