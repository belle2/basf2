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

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    /// Filter for the constuction of axial to stereo segment pairs based on simple criteria.
    class AllSegmentPairFilter : public Filter<CDCSegmentPair> {

    public:
      /**
       *  Checks if a pair of segments is a good combination.
       *  All implementation always accepts with the total number of hits as weight.
       */
      Weight operator()(const CDCSegmentPair& segmentPair) final;
    };
  }
}
