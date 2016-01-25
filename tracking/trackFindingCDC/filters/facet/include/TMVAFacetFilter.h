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
#include <tracking/trackFindingCDC/filters/facet/CDCFacetFitVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Background facet detection based on TMVA.
    class TMVAFacetFilter: public TMVAFilter<CDCFacetFitVarSet> {

    public:
      using Super = TMVAFilter<CDCFacetFitVarSet>;

    public:
      /// Constructor initialising the TMVAFilter with standard training name for this filter.
      TMVAFacetFilter()
        : Super("FacetFilter", 0.1)
      {}
    };
  }
}
