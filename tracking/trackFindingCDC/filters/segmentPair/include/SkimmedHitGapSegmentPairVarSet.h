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

#include <tracking/trackFindingCDC/filters/segmentPair/HitGapSegmentPairVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    /**
     *  Class to compute floating point variables from an axial stereo segment pair
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     *
     *  Is the same as the HitGapSegmentPairVarSet, but skimmed for non sensical combinations
     *  with a hard coded cut. To be used as a precut before proceding to a more expensive fit
     *  of the axial stereo segment pair.
     */
    class  SkimmedHitGapSegmentPairVarSet : public HitGapSegmentPairVarSet {

    private:
      /// Name of the base class
      using Super = HitGapSegmentPairVarSet;

    public:
      /// Implement the skim cut, otherwise generate and assign the variables from the segment pair
      bool extract(const CDCSegmentPair* ptrSegmentPair) final;
    };
  }
}
