/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALLAXIALSTEREOSEGMENTPAIRFILTER_H
#define ALLAXIALSTEREOSEGMENTPAIRFILTER_H

#include "BaseAxialStereoSegmentPairFilter.h"
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialStereoSegmentPair.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class AllAxialStereoSegmentPairFilter : public BaseAxialStereoSegmentPairFilter {

    public:
      /// Checks if a pair of axial segments is a good combination. All implementation always accepts the total number of hits.
      virtual CellWeight isGoodAxialStereoSegmentPair(const CDCAxialStereoSegmentPair& axialStereoSegmentPair) override final {
        const CDCAxialRecoSegment2D* ptrStartSegment = axialStereoSegmentPair.getStartSegment();
        const CDCAxialRecoSegment2D* ptrEndSegment = axialStereoSegmentPair.getEndSegment();

        if (ptrStartSegment == nullptr) {
          B2ERROR("MCAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair invoked with nullptr as start segment");
          return NOT_A_CELL;
        }

        if (ptrEndSegment == nullptr) {
          B2ERROR("MCAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair invoked with nullptr as end segment");
          return NOT_A_CELL;
        }
        const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
        const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

        return startSegment.size() + endSegment.size();
      }

    }; // end class AllAxialStereoSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //ALLAXIALSTEREOSEGMENTPAIRFILTER_H
