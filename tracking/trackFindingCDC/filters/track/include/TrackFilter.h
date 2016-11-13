/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/track/TruthTrackVarSet.h>
#include <tracking/trackFindingCDC/filters/track/BasicTrackVarSet.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    using BaseTrackFilter = Filter<CDCTrack>;

    using MCTrackFilter = MCFilter<VariadicUnionVarSet<TruthTrackVarSet, BasicTrackVarSet>>;

    using RecordingTrackFilter =
      RecordingFilter<VariadicUnionVarSet<TruthTrackVarSet, BasicTrackVarSet>>;

    using AllTrackFilter = AllFilter<BaseTrackFilter>;

    using TMVATrackFilter = TMVAFilter<BasicTrackVarSet>;

  }
}
