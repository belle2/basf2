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
    ///Class filtering the neighborhood of facets with monte carlo information
    class AllFacetRelationFilter : public BaseFacetRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseFacetRelationFilter;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return -2 for all valid combinations to accepting all facets,
       *  but compensating for overlap.
       */
      virtual Weight operator()(const CDCFacet& fromFacet,
                                const CDCFacet& toFacet) override final
      {
        // The last wire of the neighbor should not be the same as the start wire of the facet
        // The  default weight must be -2 because the overlap of the facets is two points
        // so the amount of two facets is 4 points hence the cellular automat
        // must calculate 3 + (-2) + 3 = 4 as cellstate
        // this can of course be adjusted for a more realistic information measure
        // ( together with the facet creator filter)
        return fromFacet.getStartWire() == toFacet.getEndWire() ? NAN : -2;
      }

    };
  }
}
