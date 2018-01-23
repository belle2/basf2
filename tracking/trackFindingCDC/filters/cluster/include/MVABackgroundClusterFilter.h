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

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/filters/cluster/BasicClusterVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Background cluster detection based on MVA package.
    class MVABackgroundClusterFilter
      : public MVAFilter<BasicClusterVarSet> {

    public:
      /// Type of the base class.
      using Super = MVAFilter<BasicClusterVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVABackgroundClusterFilter();
    };
  }
}
