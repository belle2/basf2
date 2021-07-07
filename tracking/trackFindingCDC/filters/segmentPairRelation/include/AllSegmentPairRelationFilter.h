/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
