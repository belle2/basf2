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

#include <tracking/trackFindingCDC/filters/segment/TruthSegmentVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;

    /**
     *  Class to compute floating point variables from a segment
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BkgTruthSegmentVarSet : public TruthSegmentVarSet {

    private:
      /// Type of the base class
      using Super = TruthSegmentVarSet;

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCSegment2D* segment) final {
        bool extracted = Super::extract(segment);
        var<named("truth")>() = not var<named("segment_is_fake_truth")>();
        return extracted;
      }
    };
  }
}
