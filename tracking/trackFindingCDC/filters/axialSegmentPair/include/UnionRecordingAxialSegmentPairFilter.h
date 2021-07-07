/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
