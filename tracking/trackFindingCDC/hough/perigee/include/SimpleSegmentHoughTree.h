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

#include <tracking/trackFindingCDC/hough/perigee/SimpleHitBasedHoughTree.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A simple hough tree taking segments as smallest units.
    template<class AInBox, size_t ... divisions>
    using SimpleSegmentHoughTree =
      SimpleHitBasedHoughTree<const CDCSegment2D*, AInBox, divisions ... >;

  }
}
