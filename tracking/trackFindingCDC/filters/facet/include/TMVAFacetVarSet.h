/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/filters/facet/FitFacetVarSet.h>
#include <tracking/trackFindingCDC/filters/facet/BendFacetVarSet.h>
#include <tracking/trackFindingCDC/filters/facet/FitlessFacetVarSet.h>
#include <tracking/trackFindingCDC/filters/facet/BasicFacetVarSet.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Variable set used in the application of tmva methods for facet filtering
    using TMVAFacetVarSet = VariadicUnionVarSet<BasicFacetVarSet,
          FitlessFacetVarSet,
          BendFacetVarSet,
          FitFacetVarSet>;

  }
}
