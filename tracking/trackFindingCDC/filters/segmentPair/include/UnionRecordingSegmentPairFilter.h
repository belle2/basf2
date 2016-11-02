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

#include <tracking/trackFindingCDC/filters/segmentPair/SegmentPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCSegmentPairs.
    class UnionRecordingSegmentPairFilter: public UnionRecordingFilter<SegmentPairFilterFactory> {

    private:
      /// Type of the base class
      using Super = UnionRecordingFilter<SegmentPairFilterFactory>;

    public:
      /// Valid names of variable sets for segment pairs.
      virtual std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for segment pairs from a name.
      virtual
      std::unique_ptr<BaseVarSet<CDCSegmentPair> >
      createVarSet(const std::string& name) const override;

    };
  }
}
