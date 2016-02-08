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

#include <tracking/trackFindingCDC/collectors/stereo_segments/StereoSegmentTrackMatcherFilter.h>
#include <tracking/trackFindingCDC/collectors/stereo_segments/StereoSegmentTrackMatcherQuadTree.h>
#include <tracking/trackFindingCDC/collectors/stereo_segments/StereoSegmentTrackAdder.h>

#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Shortcut for the used stereo segment collector for single match.
    using StereoSegmentCollectorSingleMatch = SingleMatchCollector<StereoSegmentTrackMatcherQuadTree, StereoSegmentTrackAdder>;
  }
}
