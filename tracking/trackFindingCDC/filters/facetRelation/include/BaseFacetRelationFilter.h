/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
