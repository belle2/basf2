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

#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/CDCWireHitClusterBasicVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Background cluster detection based on TMVA.
    class TMVAClusterFilter: public TMVAFilter<CDCWireHitClusterBasicVarSet> {

    public:
      using Super = TMVAFilter<CDCWireHitClusterBasicVarSet>;

    public:
      /// Constructor initialising the TMVAFilter with standard training name for this filter.
      TMVAClusterFilter() :
        TMVAFilter<CDCWireHitClusterBasicVarSet>("BackgroundHitFinder")
      {}
    };
  }
}
