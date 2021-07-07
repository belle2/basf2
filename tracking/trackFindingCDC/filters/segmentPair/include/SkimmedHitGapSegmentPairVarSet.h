/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
