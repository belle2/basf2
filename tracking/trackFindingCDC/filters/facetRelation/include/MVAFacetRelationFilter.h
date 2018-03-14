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

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/facetRelation/BasicFacetRelationVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Set of variables to be used by the mva classifier
    using MVAFacetRelationVarSet = BasicFacetRelationVarSet;

    /// Background facetRelation detection based on MVA.
    class MVAFacetRelationFilter: public MVA<BaseFacetRelationFilter> {

    private:
      /// Type of the base class
      using Super = MVA<BaseFacetRelationFilter>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVAFacetRelationFilter();

      /**
       *  Main filter method returning the weight of the facet relation.
       *  The size of the facetRelation with a small penalty depending on the mva probability.
       */
      Weight predict(const Relation<const CDCFacet>& facetRelation) final;
    };
  }
}
