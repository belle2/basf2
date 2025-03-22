/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <tracking/trackFindingCDC/filters/facet/BasicFacetVarSet.h>
#include <tracking/trackFindingCDC/filters/facet/Chi2FacetFilter.h>

#include <tracking/trackingUtilities/filters/base/MVAFilter.dcl.h>

#include <tracking/trackingUtilities/filters/base/FilterVarSet.dcl.h>

#include <tracking/trackingUtilities/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Variable set used in the application of mva methods for facet filtering
    using MVAFacetVarSet = TrackingUtilities::VariadicUnionVarSet<BasicFacetVarSet, TrackingUtilities::FilterVarSet<Chi2FacetFilter>>;

    /// Background facet detection based on MVA.
    class MVAFacetFilter: public TrackingUtilities::MVA<BaseFacetFilter> {

    public:
      /// Type of the base class
      using Super = TrackingUtilities::MVA<BaseFacetFilter>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVAFacetFilter();

      /**
       *  Main filter method returning the weight of the facet.
       *  The size of the facet with a small penalty depending on the mva probability.
       */
      TrackingUtilities::Weight predict(const CDCFacet& facet) final;
    };
  }
}
