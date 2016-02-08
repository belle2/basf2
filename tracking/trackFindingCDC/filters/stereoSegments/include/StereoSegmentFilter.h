/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentVarSet.h>
#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentTruthVarSet.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/RandomFilter.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Base filter for stereo segment - track relations.
    using BaseStereoSegmentFilter = Filter<std::pair<std::pair<const CDCRecoSegment2D*, const CDCRecoSegment3D>, const CDCTrack&>>;

    /// MC filter for stereo segment - track relations.
    using MCStereoSegmentFilter = MCFilter<StereoSegmentTruthVarSet>;

    /// Recording filter for stereo segment - track relations.
    using RecordingStereoSegmentFilter =
      RecordingFilter<VariadicUnionVarSet<StereoSegmentTruthVarSet, StereoSegmentVarSet>>;

    /// All filter for stereo segment - track relations.
    using AllStereoSegmentFilter = AllFilter<BaseStereoSegmentFilter>;

    /// Random filter for stereo segment - track relations.
    using RandomStereoSegmentFilter = RandomFilter<BaseStereoSegmentFilter>;

    /// TMVA filter for stereo segment - track relations.
    using TMVAStereoSegmentFilter = TMVAFilter<StereoSegmentVarSet>;
  }
}
