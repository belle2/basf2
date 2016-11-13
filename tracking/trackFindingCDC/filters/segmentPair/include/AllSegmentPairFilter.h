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
    /// Filter for the constuction of axial to stereo segment pairs based on simple criteria.
    class AllSegmentPairFilter : public Filter<CDCSegmentPair> {

    public:
      /** Checks if a pair of segments is a good combination.
       *  All implementation always accepts the total number of hits.
       */

      Weight operator()(const CDCSegmentPair& segmentPair) final {
        const CDCRecoSegment2D* ptrStartSegment = segmentPair.getFromSegment();
        const CDCRecoSegment2D* ptrEndSegment = segmentPair.getToSegment();

        assert(ptrStartSegment);
        assert(ptrEndSegment);

        const CDCRecoSegment2D& startSegment = *ptrStartSegment;
        const CDCRecoSegment2D& endSegment = *ptrEndSegment;
        return startSegment.size() + endSegment.size();
      }

    };
  }
}
