/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <tracking/trackFindingCDC/filters/facet/BasicFacetVarSet.h>
#include <tracking/trackFindingCDC/filters/facet/Chi2FacetFilter.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/filters/base/FilterVarSet.dcl.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Variable set used in the application of mva methods for facet filtering
    using MVAFacetVarSet = VariadicUnionVarSet<BasicFacetVarSet,
          FilterVarSet<Chi2FacetFilter>>;

    /// Background facet detection based on MVA.
    class MVAFacetFilter: public MVA<BaseFacetFilter> {

    public:
      /// Type of the base class
      using Super = MVA<BaseFacetFilter>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVAFacetFilter();

      /**
       *  Main filter method returning the weight of the facet.
       *  The size of the facet with a small penalty depending on the mva probability.
       */
      Weight predict(const CDCFacet& facet) final;
    };
  }
}
