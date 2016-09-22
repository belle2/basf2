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

#include <tracking/trackFindingCDC/filters/segmentPairRelation/SegmentPairRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered relations between segments.
    class UnionRecordingSegmentPairRelationFilter:
      public UnionRecordingFilter<SegmentPairRelationFilterFactory> {

    private:
      /// Type of the base class
      typedef  UnionRecordingFilter<SegmentPairRelationFilterFactory> Super;

    public:
      /// Valid names of variable sets for segments.
      virtual std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for segments from a name.
      virtual
      std::unique_ptr<BaseVarSet<Relation<const CDCSegmentPair> > >
      createVarSet(const std::string& name) const override;
    };
  }
}
