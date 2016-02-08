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
#include <tracking/trackFindingCDC/collectors/base/FirstMatchCollector.h>
#include <tracking/trackFindingCDC/collectors/base/BestMatchCollector.h>
#include <tracking/trackFindingCDC/collectors/base/SingleMatchCollector.h>
#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitTrackAdder.h>

#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitTrackMatcherFilter.h>
#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitTrackMatcherQuadTree.h>

#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Shortcut for the used stereo hit collector for first match.
    using StereoHitCollectorFirstMatch = FirstMatchCollector<StereoHitTrackMatcherQuadTree, StereoHitTrackAdder>;
    /// Shortcut for the used stereo hit collector for best match.
    using StereoHitCollectorBestMatch = BestMatchCollector<StereoHitTrackMatcherFilter, StereoHitTrackAdder>;
  }
}
