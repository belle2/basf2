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

#include <tracking/trackFindingCDC/filters/axialSegmentPair/AxialSegmentPairFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.dcl.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCAxialSegmentPair;

    extern template class TrackFindingCDC::UnionRecordingFilter<AxialSegmentPairFilterFactory>;

    /// Filter to record multiple chooseable variable sets for axial segment pairs
    class UnionRecordingAxialSegmentPairFilter: public UnionRecordingFilter<AxialSegmentPairFilterFactory> {

    private:
      /// Type of the base class
      using Super = UnionRecordingFilter<AxialSegmentPairFilterFactory>;

    public:
      /// Get the valid names of variable sets for axial segment pairs.
      std::vector<std::string> getValidVarSetNames() const final;

      /// Create a concrete variables set for axial segment pairs from a name.
      std::unique_ptr<BaseVarSet<CDCAxialSegmentPair> >
      createVarSet(const std::string& name) const final;
    };
  }
}
