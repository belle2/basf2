/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segment_pair/BaseSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/segment_pair/CDCSegmentPairVarSets.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCSegmentPairs.
    class RecordingSegmentPairFilter:
      public RecordingFilter<VariadicUnionVarSet<CDCSegmentPairTruthVarSet,
      CDCSegmentPairFitVarSet> > {

    private:
      /// Type of the base class.
      typedef RecordingFilter<VariadicUnionVarSet<CDCSegmentPairTruthVarSet,
              CDCSegmentPairFitVarSet> > Super;

    public:
      /// Constructor initialising the RecordingFilter with standard root file name for this filter.
      RecordingSegmentPairFilter() :
        Super("CDCSegmentPairTruthRecords.root")
      {;}

    };
  }
}
