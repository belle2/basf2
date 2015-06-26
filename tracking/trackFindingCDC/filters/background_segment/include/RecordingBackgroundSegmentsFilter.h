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

#include <tracking/trackFindingCDC/filters/background_segment/BaseBackgroundSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/background_segment/BackgroundSegmentVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class RecordingBackgroundSegmentsFilter: public RecordingFilter<BackgroundSegmentTruthVarSet> {

    public:
      /// Constructor initialising the RecordingFilter with standard root file name for this filter.
      RecordingBackgroundSegmentsFilter(const std::string& filename = "BackgroundSegmentsFilter.root") :
        RecordingFilter<BackgroundSegmentTruthVarSet>(filename)
      {;}

    };
  }
}
