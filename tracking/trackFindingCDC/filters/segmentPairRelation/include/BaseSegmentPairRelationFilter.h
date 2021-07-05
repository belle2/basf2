/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    // Guard to prevent repeated instantiations
    extern template class RelationFilter<const CDCSegmentPair>;

    /// Base class for filtering the neighborhood of axial stereo segment pairs
    class BaseSegmentPairRelationFilter : public RelationFilter<const CDCSegmentPair> {

    public:
      /// Default constructor
      BaseSegmentPairRelationFilter();

      /// Default destructor
      ~BaseSegmentPairRelationFilter();

      /// Returns the segment pairs form the range that continue on the to site of the given segment pair.
      std::vector<const CDCSegmentPair*> getPossibleTos(
        const CDCSegmentPair* from,
        const std::vector<const CDCSegmentPair*>& segmentPairs) const final;
    };
  }
}
