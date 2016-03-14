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

#include <tracking/trackFindingCDC/filters/segment_pair/BaseSegmentPairFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of axial to stereo segment pairs based on simple criteria.
    class AllSegmentPairFilter : public Filter<CDCSegmentPair> {

    public:
      /** Checks if a pair of segments is a good combination.
       *  All implementation always accepts the total number of hits.
       */
      virtual
      CellWeight operator()(const CDCSegmentPair& segmentPair) override final
      {
        const CDCRecoSegment2D* ptrStartSegment = segmentPair.getStartSegment();
        const CDCRecoSegment2D* ptrEndSegment = segmentPair.getEndSegment();

        assert(ptrStartSegment);
        assert(ptrEndSegment);

        const CDCRecoSegment2D& startSegment = *ptrStartSegment;
        const CDCRecoSegment2D& endSegment = *ptrEndSegment;
        return startSegment.size() + endSegment.size();
      }

    }; // end class AllSegmentPairFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
