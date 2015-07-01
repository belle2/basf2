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

#include <tracking/trackFindingCDC/filters/segment/AdvancedRecoSegment2DVarSet.h>
#include <tracking/trackFindingCDC/filters/segment/CDCRecoSegment2DTruthVarSet.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    using BaseNewSegmentsFilter = Filter<CDCRecoSegment2D>;

    using MCNewSegmentsFilter = MCFilter<VariadicUnionVarSet<CDCRecoSegment2DTruthVarSet, AdvancedCDCRecoSegment2DVarSet>>;

    using RecordingNewSegmentsFilter =
      RecordingFilter<VariadicUnionVarSet<CDCRecoSegment2DTruthVarSet, AdvancedCDCRecoSegment2DVarSet>>;

    using AllNewSegmentsFilter = AllFilter<BaseNewSegmentsFilter>;

    using TMVANewSegmentsFilter = TMVAFilter<AdvancedCDCRecoSegment2DVarSet>;
  }
}
