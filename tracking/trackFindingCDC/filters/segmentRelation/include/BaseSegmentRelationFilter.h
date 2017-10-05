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

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/Range.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;

    // Guard to prevent repeated instatiations
    extern template class Filter<Relation<const CDCSegment2D> >;

    /// Base class for filtering the neighborhood of segments
    class BaseSegmentRelationFilter : public Filter<Relation<const CDCSegment2D>> {

    public:
      /// Default constructor
      BaseSegmentRelationFilter();

      /// Default destructor
      virtual ~BaseSegmentRelationFilter();

      /// Returns the full range of segments.
      template<class ASegmentIterator>
      Range<ASegmentIterator>
      getPossibleNeighbors(const CDCSegment2D& segment,
                           const ASegmentIterator& itBegin,
                           const ASegmentIterator& itEnd) const
      {
        std::pair<ASegmentIterator, ASegmentIterator> sameSuperClusterItPair =
          std::equal_range(itBegin, itEnd, segment, std::less<CDCSegment2D>());
        return Range<ASegmentIterator>(sameSuperClusterItPair.first,
                                       sameSuperClusterItPair.second);
      }

      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all segment neighbors.
       */
      virtual Weight operator()(const CDCSegment2D& from, const CDCSegment2D& to);

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<const CDCSegment2D>& relation) override;
    };
  }
}
