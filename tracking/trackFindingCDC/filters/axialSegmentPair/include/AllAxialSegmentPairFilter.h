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

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter accepting all axial to axial segment pairs.
    class AllAxialSegmentPairFilter : public BaseAxialSegmentPairFilter {

      /// Checks if a pair of axial segments is a good combination
      Weight operator()(const CDCAxialSegmentPair& axialSegmentPair) final;
    };

  }
}
