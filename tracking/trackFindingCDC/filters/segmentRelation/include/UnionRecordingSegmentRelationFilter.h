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

#include <tracking/trackFindingCDC/filters/segmentRelation/SegmentRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered relations between segments.
    class UnionRecordingSegmentRelationFilter:
      public UnionRecordingFilter<SegmentRelationFilterFactory> {

    private:
      /// Type of the base class
      using Super = UnionRecordingFilter<SegmentRelationFilterFactory>;

    public:
      /// Valid names of variable sets for segments.
      std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for segments from a name.

      std::unique_ptr<BaseVarSet<Relation<const CDCRecoSegment2D> > >
      createVarSet(const std::string& name) const override;
    };
  }
}
