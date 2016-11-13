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
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/TMVAFacetRelationVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Background facetRelation detection based on TMVA.
    class TMVAFacetRelationFilter: public TMVA<BaseFacetRelationFilter> {

    public:
      /// Type of the base class
      using Super = TMVA<BaseFacetRelationFilter>;

    public:
      /// Constructor initialising the TMVAFilter with standard training name for this filter.
      TMVAFacetRelationFilter()
        : Super(makeUnique<TMVAFacetRelationVarSet>(),
                "FacetRelationFilter",
                -2.19)
      {}

    public:
      /**
       *  Main filter method returning the weight of the facet relation.
       *  The size of the facetRelation with a small penalty depending on the tmva probability.
       */
      Weight predict(const Relation<const CDCFacet>& facetRelation) final {
        return -2 - 0.2 * (1 - Super::predict(facetRelation));
      }

    };
  }
}
