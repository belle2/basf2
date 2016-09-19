/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/trackFindingCDC/filters/segmentPair/BasicSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/SkimmedHitGapSegmentPairVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of segment pairs based on simple criteria without the common fit.
    class MVAFeasibleSegmentPairFilter :
      public MVAFilter<VariadicUnionVarSet<BasicSegmentPairVarSet, SkimmedHitGapSegmentPairVarSet> > {

    private:
      /// Type of the base class
      using Super = MVAFilter<VariadicUnionVarSet<BasicSegmentPairVarSet, SkimmedHitGapSegmentPairVarSet> >;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVAFeasibleSegmentPairFilter()
        : Super("tracking/data/FeasibleSegmentPairFilter.weights.xml", 0.004)
      {
      }

    }; // end class MVAFeasibleSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
