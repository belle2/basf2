/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/segmentPair/MVAFeasibleSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/segmentPair/BasicSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/FitlessSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/FitSegmentPairVarSet.h>

#include <tracking/trackingUtilities/filters/base/MVAFilter.dcl.h>

#include <tracking/trackingUtilities/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    using MVARealisticSegmentPairVarSet =
      TrackingUtilities::VariadicUnionVarSet<BasicSegmentPairVarSet, FitlessSegmentPairVarSet, FitSegmentPairVarSet>;

    /// Final filter for the construction of segment pairs.
    class MVARealisticSegmentPairFilter : public TrackingUtilities::MVAFilter<MVARealisticSegmentPairVarSet> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::MVAFilter<MVARealisticSegmentPairVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticSegmentPairFilter();

      /// Function to object for its signalness
      TrackingUtilities::Weight operator()(const CDCSegmentPair& segmentPair) final;

    private:
      /// Feasibility filter applied first before invoking the main cut
      MVAFeasibleSegmentPairFilter m_feasibleSegmentPairFilter;
    };
  }
}
