/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentRelation/SegmentRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;

    /// Filter to record multiple chooseable variable sets for segment relations
    class UnionRecordingSegmentRelationFilter : public UnionRecordingFilter<SegmentRelationFilterFactory> {

    private:
      /// Type of the base class
      using Super = UnionRecordingFilter<SegmentRelationFilterFactory>;

    public:
      /// Get the valid names of variable sets for segment relations.
      std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for segment relations from a name.
      std::unique_ptr<BaseVarSet<Relation<const CDCSegment2D> > >
      createVarSet(const std::string& name) const override;
    };
  }
}
