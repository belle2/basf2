/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/facetRelation/BasicFacetRelationVarSet.h>

#include <tracking/trackingUtilities/filters/base/MVAFilter.dcl.h>

#include <tracking/trackingUtilities/utilities/Relation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Set of variables to be used by the mva classifier
    using MVAFacetRelationVarSet = BasicFacetRelationVarSet;

    /// Background facetRelation detection based on MVA.
    class MVAFacetRelationFilter: public TrackingUtilities::MVA<BaseFacetRelationFilter> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::MVA<BaseFacetRelationFilter>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVAFacetRelationFilter();

      /**
       *  Main filter method returning the weight of the facet relation.
       *  The size of the facetRelation with a small penalty depending on the mva probability.
       */
      TrackingUtilities::Weight predict(const TrackingUtilities::Relation<const CDCFacet>& facetRelation) final;
    };
  }
}
