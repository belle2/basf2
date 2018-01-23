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

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/MVAFeasibleAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BasicAxialSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/FitlessAxialSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/FitAxialSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Final filter for the constuction of segment pairs.
    class MVARealisticAxialSegmentPairFilter : public MVA<BaseAxialSegmentPairFilter> {

    private:
      /// Type of the base class
      using Super = MVA<BaseAxialSegmentPairFilter>;

      /// Type of the VarSet the filter is working on
      using VarSet = VariadicUnionVarSet<BasicAxialSegmentPairVarSet,
            FitlessAxialSegmentPairVarSet,
            FitAxialSegmentPairVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticAxialSegmentPairFilter();

      /// Tell Root to look at this operator
      using Super::operator();

    private:
      /// Function to object for its signalness
      Weight operator()(const CDCAxialSegmentPair& axialSegmentPair) override;

    private:
      /// Feasibility filter applied first before invoking the main cut
      MVAFeasibleAxialSegmentPairFilter m_feasibleAxialSegmentPairFilter;
    };
  }
}
