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

#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/segment_train/SegmentTrainTruthVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class RecordingSegmentTrainFilter: public RecordingFilter<SegmentTrainTruthVarSet> {

    public:
      /// Constructor initialising the RecordingFilter with standard root file name for this filter.
      RecordingSegmentTrainFilter() :
        RecordingFilter<SegmentTrainTruthVarSet>("SegmentTrainFilter.root")
      {
        setParameter("returned_cell_weight", "1.0");
      }

    };
  }
}
