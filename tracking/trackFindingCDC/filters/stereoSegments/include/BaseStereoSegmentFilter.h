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

#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoSegment3D;

    /// Base filter for stereo segment - track relations.
    using BaseStereoSegmentFilter = Filter<std::pair<const CDCRecoSegment3D*, const CDCTrack* >>;
  }
}
