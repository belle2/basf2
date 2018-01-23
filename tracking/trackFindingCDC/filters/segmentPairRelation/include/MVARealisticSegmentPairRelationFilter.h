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

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BasicSegmentPairRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/FitSegmentPairRelationVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Final filter for the constuction of segment pairs.
    class MVARealisticSegmentPairRelationFilter : public MVA<BaseSegmentPairRelationFilter> {

    private:
      /// Type of the base class
      using Super = MVA<BaseSegmentPairRelationFilter>;

      /// Set of variables used in this filter
      using VarSet = VariadicUnionVarSet <
                     BasicSegmentPairRelationVarSet,
                     FitSegmentPairRelationVarSet >;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVARealisticSegmentPairRelationFilter();
    };
  }
}
