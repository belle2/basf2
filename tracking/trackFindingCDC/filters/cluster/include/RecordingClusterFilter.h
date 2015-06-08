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

#include <tracking/trackFindingCDC/filters/cluster/CDCWireHitClusterBkgTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/cluster/CDCWireHitClusterBasicVarSet.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCWireHitClusters.
    class RecordingClusterFilter:
      public RecordingFilter<VariadicUnionVarSet<CDCWireHitClusterBkgTruthVarSet,
      CDCWireHitClusterBasicVarSet> > {

    private:
      /// Type of the base class
      typedef RecordingFilter<VariadicUnionVarSet<
      CDCWireHitClusterBkgTruthVarSet,
      CDCWireHitClusterBasicVarSet
      > > Super;

    public:
      /// Constructor initialising the RecordingFilter with standard root file name for this filter.
      RecordingClusterFilter() : Super("BackgroundHitFinder.root")
      {;}

    };
  }
}
