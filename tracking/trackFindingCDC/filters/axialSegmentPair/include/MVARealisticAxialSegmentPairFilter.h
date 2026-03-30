/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/MVAFeasibleAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BasicAxialSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/FitlessAxialSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/FitAxialSegmentPairVarSet.h>

#include <tracking/trackingUtilities/filters/base/MVAFilter.dcl.h>

#include <tracking/trackingUtilities/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Final filter for the construction of segment pairs.
    class MVARealisticAxialSegmentPairFilter : public TrackingUtilities::MVA<BaseAxialSegmentPairFilter> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::MVA<BaseAxialSegmentPairFilter>;

      /// Type of the TrackingUtilities::VarSet the filter is working on
      using VarSet = TrackingUtilities::VariadicUnionVarSet<BasicAxialSegmentPairVarSet,
            FitlessAxialSegmentPairVarSet,
            FitAxialSegmentPairVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticAxialSegmentPairFilter();

      /// Tell Root to look at this operator
      using Super::operator();

    private:
      /// Function to object for its signalness
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCAxialSegmentPair& axialSegmentPair) override;

    private:
      /// Feasibility filter applied first before invoking the main cut
      MVAFeasibleAxialSegmentPairFilter m_feasibleAxialSegmentPairFilter;
    };
  }
}
