/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/filters/base/RelationFilter.dcl.h>

#include <vector>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCFacet;
  }
  namespace TrackFindingCDC {

    // Guard to prevent repeated instantiations
    // extern template class TrackingUtilities::RelationFilter<const CDCFacet>;

    /// Base class for filtering the neighborhood of facets.
    class BaseFacetRelationFilter : public TrackingUtilities::RelationFilter<const TrackingUtilities::CDCFacet> {

    public:
      /// Default constructor
      BaseFacetRelationFilter();

      /// Default destructor
      ~BaseFacetRelationFilter();

      /**
       *  Returns the selection of facets covering the range of possible neighboring
       *  facets of the given facet out of the sorted range given by the two other arguments.
       */
      std::vector<const TrackingUtilities::CDCFacet*> getPossibleTos(
        const TrackingUtilities::CDCFacet* from,
        const std::vector<const TrackingUtilities::CDCFacet*>& facets) const final;
    };
  }
}
