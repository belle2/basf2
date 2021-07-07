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
    class CDCSegmentTriple;

    // Guard to prevent repeated instantiations
    extern template class RelationFilter<const CDCSegmentTriple>;

    /// Base class for filtering the neighborhood of segment triples
    class BaseSegmentTripleRelationFilter : public RelationFilter<const CDCSegmentTriple> {

    public:
      /// Default constructor
      BaseSegmentTripleRelationFilter();

      /// Default destructor
      ~BaseSegmentTripleRelationFilter();

      /// Returns the segment triples form the range that continue on the to site of the given segment triple.
      std::vector<const CDCSegmentTriple*> getPossibleTos(
        const CDCSegmentTriple* from,
        const std::vector<const CDCSegmentTriple*>& segmentTriples) const final;
    };
  }
}
