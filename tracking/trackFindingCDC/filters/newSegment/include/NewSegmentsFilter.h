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

#include <tracking/trackFindingCDC/filters/segment/AdvancedSegmentVarSet.h>
#include <tracking/trackFindingCDC/filters/segment/TruthSegmentVarSet.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
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
      virtual bool extract(const CDCRecoSegment2D* segment) override final
      {
        Super::extract(segment);

        var<named("truth")>() = var<named("segment_is_new_track_truth")>();
        return true;
      }
    };

    using BaseNewSegmentsFilter = Filter<CDCRecoSegment2D>;

    using MCNewSegmentsFilter =
      MCFilter<VariadicUnionVarSet<NewSegmentTruthVarSet, AdvancedSegmentVarSet>>;

    using RecordingNewSegmentsFilter =
      RecordingFilter<VariadicUnionVarSet<NewSegmentTruthVarSet, AdvancedSegmentVarSet> >;

    using AllNewSegmentsFilter = AllFilter<BaseNewSegmentsFilter>;

    using TMVANewSegmentsFilter = TMVAFilter<AdvancedSegmentVarSet>;
  }
}
