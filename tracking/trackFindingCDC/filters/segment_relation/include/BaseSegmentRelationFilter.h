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

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/ca/NeighborWeight.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {
    ///Base class for filtering the neighborhood of segments

    template<>
    class Filter<Relation<CDCRecoSegment2D>>  :
                                            public FilterBase<Relation<CDCRecoSegment2D>> {

    public:
      /// Returns the full range of segments.
      template<class ACDCRecoSegment2DIterator>
      boost::iterator_range<ACDCRecoSegment2DIterator>
      getPossibleNeighbors(const CDCRecoSegment2D& /* segment */,
                           const ACDCRecoSegment2DIterator& itBegin,
                           const ACDCRecoSegment2DIterator& itEnd) const
      {
        return boost::iterator_range<ACDCRecoSegment2DIterator>(itBegin, itEnd);
      }

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NOT_A_NEIGHBOR to reject all segment neighbors.
       */
      virtual NeighborWeight operator()(const CDCRecoSegment2D& /* from */,
                                        const CDCRecoSegment2D& /* to */)
      {
        return NOT_A_NEIGHBOR;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      virtual CellWeight operator()(const Relation<CDCRecoSegment2D>& relation) override
      {
        const CDCRecoSegment2D* ptrFrom = relation.first;
        const CDCRecoSegment2D* ptrTo = relation.second;
        if (ptrFrom == ptrTo) return NOT_A_NEIGHBOR; // Prevent relation to same.
        if (not ptrFrom or not ptrTo) return NOT_A_NEIGHBOR;
        return operator()(*ptrFrom, *ptrTo);
      }

      /// Legacy method
      virtual NeighborWeight isGoodNeighbor(const CDCRecoSegment2D& from,
                                            const CDCRecoSegment2D& to)
      {
        return operator()(from, to);
      }
    }; // end class

    /// Alias for the base class for filtering the neighborhood of segments.
    typedef Filter<Relation<CDCRecoSegment2D>>  BaseSegmentRelationFilter;

  } //end namespace TrackFindingCDC
} //end namespace Belle2
