/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALLAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H
#define ALLAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H

#include "BaseAxialStereoSegmentPairNeighborChooser.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Neighbor chooser that lets all possible combinations pass.
    class AllAxialStereoSegmentPairNeighborChooser : public BaseAxialStereoSegmentPairNeighborChooser {

    public:
      virtual NeighborWeight isGoodNeighbor(const CDCAxialStereoSegmentPair&,
                                            const CDCAxialStereoSegmentPair& neighborPair) override final {
        return  -neighborPair.getStartSegment()->size();
      }
    }; // end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // ALLAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H
