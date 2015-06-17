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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/algorithms/NeighborWeight.h>
#include <tracking/trackFindingCDC/algorithms/Relation.h>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>
#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {
    ///Base class for filtering the neighborhood of tracks
    template<>
    class Filter<Relation<CDCTrack> > :
      public FilterBase<Relation<CDCTrack> > {

    public:
      /// Returns the full range of tracks.
      template<class CDCTrackIterator>
      boost::iterator_range<CDCTrackIterator>
      getPossibleNeighbors(const CDCTrack& /* track */,
                           const CDCTrackIterator& itBegin,
                           const CDCTrackIterator& itEnd) const
      {
        return boost::iterator_range<CDCTrackIterator>(itBegin, itEnd);
      }

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NOT_A_NEIGHBOR to reject all track neighbors.
       */
      virtual NeighborWeight operator()(const CDCTrack& /* from */,
                                        const CDCTrack& /* to */)
      {
        return NOT_A_NEIGHBOR;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      virtual CellWeight operator()(const Relation<CDCTrack>& relation) override
      {
        const CDCTrack* ptrFrom = relation.first;
        const CDCTrack* ptrTo = relation.second;
        if (ptrFrom == ptrTo) return NOT_A_NEIGHBOR; // Prevent relation to same.
        if (not ptrFrom or not ptrTo) return NOT_A_NEIGHBOR;
        return operator()(*ptrFrom, *ptrTo);
      }

      /// Legacy method
      virtual NeighborWeight isGoodNeighbor(const CDCTrack& from,
                                            const CDCTrack& to)
      {
        return operator()(from, to);
      }
    }; // end class

    /// Alias for the base class for filtering the neighborhood of tracks.
    typedef Filter<Relation<CDCTrack> >  BaseTrackRelationFilter;

  } //end namespace TrackFindingCDC
} //end namespace Belle2
