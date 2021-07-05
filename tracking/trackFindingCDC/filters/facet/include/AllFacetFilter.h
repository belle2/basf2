/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Filter for the constuction of good facets based on simple criterions.
    class AllFacetFilter : public BaseFacetFilter {

    public:
      /// Main filter method returning the weight of the facet. Returns 3 to accept all facets.
      Weight operator()(const CDCFacet& facet __attribute__((unused))) final {
        return 3.0;
      }
    };
  }
}
