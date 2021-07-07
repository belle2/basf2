/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BasicAxialSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/HitGapAxialSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    extern template class TrackFindingCDC::MVA<BaseAxialSegmentPairFilter>;

    /// Filter for the constuction of segment pairs based on simple criteria without the common fit.
    class MVAFeasibleAxialSegmentPairFilter : public MVA<BaseAxialSegmentPairFilter> {

    private:
      /// Type of the base class
      using Super = MVA<BaseAxialSegmentPairFilter>;

      /// Type of the VarSet the filter is working on
      using VarSet = VariadicUnionVarSet<BasicAxialSegmentPairVarSet, HitGapAxialSegmentPairVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVAFeasibleAxialSegmentPairFilter();

      /// Tell Root to look at this operator
      using Super::operator();
    };
  }
}
