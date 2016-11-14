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

#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <vector>
#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
    using BaseSegmentTrackFilter = Filter<std::pair<const CDCRecoSegment2D*, const CDCTrack*>>;
  }
}
