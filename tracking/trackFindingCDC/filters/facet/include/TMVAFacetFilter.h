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
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/facet/TMVAFacetVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Background facet detection based on TMVA.
    class TMVAFacetFilter: public TMVAFilter<TMVAFacetVarSet> {

    public:
      using Super = TMVAFilter<FitFacetVarSet>;

    public:
      /// Constructor initialising the TMVAFilter with standard training name for this filter.
      TMVAFacetFilter()
        : Super("FacetFilter", 2.82)
      {}

    public:
      /**
       *  Main filter method returning the weight of the facet.
       *  The size of the facet with a small penalty depending on the tmva probability.
       */
      virtual Weight operator()(const CDCFacet& facet) override final
      {
        return 3 - 0.2 * (1 - Super::operator()(facet));
      }

    };
  }
}
