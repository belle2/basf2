/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
