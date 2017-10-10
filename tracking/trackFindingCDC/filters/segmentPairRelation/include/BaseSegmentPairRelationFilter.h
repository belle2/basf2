/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
