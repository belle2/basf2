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

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    using BaseSegmentInformationListTrackFilter = Filter<std::pair<std::vector<SegmentInformation*>, const CDCTrack*>>;

    using MCSegmentInformationListTrackFilter = MCFilter<VariadicUnionVarSet<SegmentTrainTruthVarSet, SegmentTrainVarSet>>;

    using RecordingSegmentInformationListTrackFilter =
      RecordingFilter<VariadicUnionVarSet<SegmentTrainTruthVarSet, SegmentTrainVarSet>>;

    using AllSegmentInformationListTrackFilter = AllFilter<BaseSegmentInformationListTrackFilter>;

    using TMVASegmentInformationListTrackFilter = TMVAFilter<SegmentTrainVarSet>;
  }
}
