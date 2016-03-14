/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitVarSet.h>
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitTruthVarSet.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/RandomFilter.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    using BaseStereoHitFilter = Filter<std::pair<const CDCRecoHit3D*, const CDCTrack*>>;

    using MCStereoHitFilter = MCFilter<VariadicUnionVarSet<StereoHitTruthVarSet, StereoHitVarSet>>;

    using RecordingStereoHitFilter =
      RecordingFilter<VariadicUnionVarSet<StereoHitTruthVarSet, StereoHitVarSet>>;

    using AllStereoHitFilter = AllFilter<BaseStereoHitFilter>;

    using SimpleStereoHitFilter = NamedChoosableVarSetFilter<StereoHitTruthVarSet>;

    using RandomStereoHitFilter = RandomFilter<BaseStereoHitFilter>;
  }
}
