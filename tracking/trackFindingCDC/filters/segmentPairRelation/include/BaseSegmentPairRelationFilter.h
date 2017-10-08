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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <algorithm>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    // Guard to prevent repeated instantiations
    extern template class Filter<Relation<const CDCSegmentPair> >;

    /// Base class for filtering the neighborhood of axial stereo segment pairs
    class BaseSegmentPairRelationFilter  : public Filter<Relation<const CDCSegmentPair>> {

    public:
      /// Returns the segment pairs form the range that continue on the to site of the given segment pair.
      std::vector<const CDCSegmentPair*> getPossibleNeighbors(
        const CDCSegmentPair* segmentPair,
        const std::vector<const CDCSegmentPair*>::const_iterator& itBegin,
        const std::vector<const CDCSegmentPair*>::const_iterator& itEnd) const
      {
        assert(std::is_sorted(itBegin, itEnd, LessOf<Deref>()) && "Expected segment pairs to be sorted");
        const CDCSegment2D* toSegment = segmentPair->getToSegment();

        ConstVectorRange<const CDCSegmentPair*> neighbors{
          std::equal_range(itBegin, itEnd, &toSegment, LessOf<Deref>())};
        return {neighbors.begin(),  neighbors.end()};
      }

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all axial stereo segment pair neighbors.
       */
      virtual Weight operator()(const CDCSegmentPair& from  __attribute__((unused)),
                                const CDCSegmentPair& to  __attribute__((unused)))
      {
        return 1;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      Weight operator()(const Relation<const CDCSegmentPair>& relation) override
      {
        const CDCSegmentPair* ptrFrom(relation.first);
        const CDCSegmentPair* ptrTo(relation.second);
        if ((ptrFrom == nullptr) or (ptrTo == nullptr)) return NAN;
        return this->operator()(*ptrFrom, *ptrTo);
      }
    };
  }
}
