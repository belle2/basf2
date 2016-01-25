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

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Variable set used in the application of tmva methods for facet filtering
    using TMVAFacetVarSet = FitFacetVarSet;

  }
}
