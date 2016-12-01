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

#include <tracking/trackFindingCDC/filters/segmentPair/MVAFeasibleSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/segmentPair/BasicSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/FitlessSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/FitSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Final filter for the constuction of segment pairs.
    class MVARealisticSegmentPairFilter :
      public MVAFilter<VariadicUnionVarSet<
      BasicSegmentPairVarSet,
      FitlessSegmentPairVarSet,
      FitSegmentPairVarSet> > {

    private:
      /// Type of the base class
      using Super = MVAFilter<VariadicUnionVarSet<
                    BasicSegmentPairVarSet,
                    FitlessSegmentPairVarSet,
                    FitSegmentPairVarSet> >;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticSegmentPairFilter();

      /// Function to object for its signalness
      Weight operator()(const CDCSegmentPair& segmentPair) override;

    private:
      /// Feasibility filter applied first before invoking the main cut
      MVAFeasibleSegmentPairFilter m_feasibleSegmentPairFilter;
    };
  }
}
