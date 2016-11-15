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

#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Base class for filtering the neighborhood of axial stereo segment pairs
    class BaseSegmentPairRelationFilter  : public Filter<Relation<const CDCSegmentPair>> {

    public:
      /// Returns a two iterator range covering the range of possible neighboring axial stereo segment pairs of the given axial stereo segment pair out of the sorted range given by the two other argumets.
      template<class ACDCSegmentPairIterator>
      boost::iterator_range<ACDCSegmentPairIterator>
      getPossibleNeighbors(const CDCSegmentPair& segmentPair,
                           const ACDCSegmentPairIterator& itBegin,
                           const ACDCSegmentPairIterator& itEnd) const
      {

        const CDCSegment2D* ptrEndSegment = segmentPair.getToSegment();
        if (not ptrEndSegment) boost::iterator_range<ACDCSegmentPairIterator>(itEnd, itEnd);

        std::pair<ACDCSegmentPairIterator, ACDCSegmentPairIterator> itPairPossibleNeighbors = std::equal_range(itBegin,
            itEnd, ptrEndSegment);
        return boost::iterator_range<ACDCSegmentPairIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);
      }

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all axial stereo segment pair neighbors.
       */
      virtual Weight operator()(const CDCSegmentPair& from  __attribute__((unused)),
                                const CDCSegmentPair& to  __attribute__((unused)))
      {
        return NAN;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      Weight operator()(const Relation<const CDCSegmentPair>& relation) override
      {
        const CDCSegmentPair* ptrFrom(relation.first);
        const CDCSegmentPair* ptrTo(relation.second);
        if (not ptrFrom or not ptrTo) return NAN;
        return operator()(*ptrFrom, *ptrTo);
      }
    };
  }
}
