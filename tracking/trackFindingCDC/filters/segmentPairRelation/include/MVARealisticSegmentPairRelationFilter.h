/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BasicSegmentPairRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/FitSegmentPairRelationVarSet.h>

#include <tracking/trackingUtilities/filters/base/MVAFilter.dcl.h>

#include <tracking/trackingUtilities/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Final filter for the construction of segment pairs.
    class MVARealisticSegmentPairRelationFilter : public TrackingUtilities::MVA<BaseSegmentPairRelationFilter> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::MVA<BaseSegmentPairRelationFilter>;

      /// Set of variables used in this filter
      using VarSet = TrackingUtilities::VariadicUnionVarSet <
                     BasicSegmentPairRelationVarSet,
                     FitSegmentPairRelationVarSet >;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticSegmentPairRelationFilter();
    };
  }
}
