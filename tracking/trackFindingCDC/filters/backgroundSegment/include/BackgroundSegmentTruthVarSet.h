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

#include <tracking/trackFindingCDC/filters/segment/CDCRecoSegment2DTruthVarSet.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Truth var set for finding background segments.
    class BackgroundSegmentTruthVarSet : public CDCRecoSegment2DTruthVarSet {

    public:
      /// Construct the peeler.
      explicit BackgroundSegmentTruthVarSet() : CDCRecoSegment2DTruthVarSet() { }

      /// Generate and assign the variables from the cluster
      virtual bool extract(const CDCRecoSegment2D* segment) override final
      {
        CDCRecoSegment2DTruthVarSet::extract(segment);

        var<named("truth")>() = var<named("segment_is_fake_truth")>();
        return true;
      }
    };
  }
}
