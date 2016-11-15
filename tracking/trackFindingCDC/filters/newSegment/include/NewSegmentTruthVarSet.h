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
     *  Class that computes floating point variables from a segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class NewSegmentTruthVarSet : public TruthSegmentVarSet {

    private:
      /// Type of the base class
      using Super = TruthSegmentVarSet;

    public:
      /// Generate and assign the variables from the cluster
      bool extract(const CDCSegment2D* segment) final {
        bool extracted = Super::extract(segment);
        var<named("truth")>() = var<named("segment_is_new_track_truth")>();
        return extracted;
      }
    };
  }
}
