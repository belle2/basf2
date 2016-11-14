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
#include <tracking/trackFindingCDC/filters/backgroundSegment/BackgroundSegmentTruthVarSet.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Base Filter for filtering out Background Segments, but rejects all segments.
    using BaseBackgroundSegmentFilter = Filter<CDCRecoSegment2D>;

    /// MC Filter for filtering out Background Segments.
    using MCBackgroundSegmentFilter = MCFilter<VariadicUnionVarSet<BackgroundSegmentTruthVarSet,
          AdvancedSegmentVarSet>>;

    /// Recording Filter for filtering out Background Segments.
    using RecordingBackgroundSegmentFilter =
      RecordingFilter<VariadicUnionVarSet<BackgroundSegmentTruthVarSet, AdvancedSegmentVarSet>>;

    /// Filter for filtering out Background Segments, but accepts all segments.
    using AllBackgroundSegmentFilter = AllFilter<BaseBackgroundSegmentFilter>;

    /// TMVA Filter for filtering out Background Segments.
    using TMVABackgroundSegmentFilter = TMVAFilter<AdvancedSegmentVarSet>;
  }
}
