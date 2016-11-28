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

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Base class for filtering the neighborhood of tracks
    class BaseTrackRelationFilter : public Filter<Relation<const CDCTrack> > {

    public:
      /// Returns the full range of tracks.
      template<class ACDCTrackIterator>
      boost::iterator_range<ACDCTrackIterator>
      getPossibleNeighbors(const CDCTrack& track  __attribute__((unused)),
                           const ACDCTrackIterator& itBegin,
                           const ACDCTrackIterator& itEnd) const
      {
        return boost::iterator_range<ACDCTrackIterator>(itBegin, itEnd);
      }

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all track neighbors.
       */
      virtual Weight operator()(const CDCTrack& from  __attribute__((unused)),
                                const CDCTrack& to  __attribute__((unused)))
      {
        return NAN;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      Weight operator()(const Relation<const CDCTrack>& relation) override
      {
        const CDCTrack* ptrFrom(relation.first);
        const CDCTrack* ptrTo(relation.second);
        if (ptrFrom == ptrTo) return NAN; // Prevent relation to same.
        if (not ptrFrom or not ptrTo) return NAN;
        return operator()(*ptrFrom, *ptrTo);
      }
    };
  }
}
