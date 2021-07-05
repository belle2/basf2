/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/filters/segmentRelation/BasicSegmentRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/HitGapSegmentRelationVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of segment pairs based on simple criteria without the common fit.
    class MVAFeasibleSegmentRelationFilter : public MVA<BaseSegmentRelationFilter> {

    private:
      /// Type of the base class
      using Super = MVA<BaseSegmentRelationFilter>;

      /// Type of the VarSet the filter is working on
      using VarSet = VariadicUnionVarSet<BasicSegmentRelationVarSet, HitGapSegmentRelationVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVAFeasibleSegmentRelationFilter();

      /// Tell Root to look at this operator
      using Super::operator();
    };
  }
}
