/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of segment relations based on MC information.
    class MCSegmentRelationFilter : public MCSymmetricFilterMixin<BaseSegmentRelationFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetricFilterMixin<BaseSegmentRelationFilter >;

    public:
      /// Constructor
      MCSegmentRelationFilter(bool allowReverse = true);

      /// Tell Root to look at this operator
      using Super::operator();

    private:
      /// Checks if a segment relation is a good combination.
      Weight operator()(const CDCSegment2D& fromSegment, const CDCSegment2D& toSegment) final;
    };
  }
}
