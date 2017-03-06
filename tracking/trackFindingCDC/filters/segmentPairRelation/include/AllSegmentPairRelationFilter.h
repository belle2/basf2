/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Relation filter that lets all possible combinations pass.
    class AllSegmentPairRelationFilter : public BaseSegmentPairRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseSegmentPairRelationFilter;

    public:
      /// Implementation accepting all possible neighbors. Weight subtracts the overlap penalty.
      Weight operator()(const CDCSegmentPair& fromSegmentPair, const CDCSegmentPair& toSegmentPair) final;
    };
  }
}
