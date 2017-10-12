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

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    /// Filter to record multiple chooseable variable sets for segment pairs
    class UnionRecordingSegmentPairFilter: public UnionRecordingFilter<SegmentPairFilterFactory> {

    private:
      /// Type of the base class
      using Super = UnionRecordingFilter<SegmentPairFilterFactory>;

    public:
      /// Get the valid names of variable sets for segment pairs.
      std::vector<std::string> getValidVarSetNames() const final;

      /// Create a concrete variables set for segment pairs from a name.
      std::unique_ptr<BaseVarSet<CDCSegmentPair> >
      createVarSet(const std::string& name) const final;
    };
  }
}
