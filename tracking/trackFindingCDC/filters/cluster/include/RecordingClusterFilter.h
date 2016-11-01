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

#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>

#include <tracking/trackFindingCDC/filters/cluster/BkgTruthClusterVarSet.h>
#include <tracking/trackFindingCDC/filters/cluster/BasicClusterVarSet.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCWireHitClusters for a training sample for the background filter
    using RecordingClusterFilter =
      RecordingFilter<VariadicUnionVarSet<BkgTruthClusterVarSet,
      BasicClusterVarSet> >;

  }
}
