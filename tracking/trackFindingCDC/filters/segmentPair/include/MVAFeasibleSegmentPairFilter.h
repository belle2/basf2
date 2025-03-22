/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPair/BasicSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/SkimmedHitGapSegmentPairVarSet.h>

#include <tracking/trackingUtilities/filters/base/MVAFilter.dcl.h>

#include <tracking/trackingUtilities/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    using MVAFeasibleSegmentPairVarSet =
      TrackingUtilities::VariadicUnionVarSet<BasicSegmentPairVarSet, SkimmedHitGapSegmentPairVarSet>;

    /// Filter for the construction of segment pairs based on simple criteria without the common fit.
    class MVAFeasibleSegmentPairFilter : public TrackingUtilities::MVAFilter<MVAFeasibleSegmentPairVarSet> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::MVAFilter<MVAFeasibleSegmentPairVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVAFeasibleSegmentPairFilter();
    };
  }
}
