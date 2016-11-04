/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <boost/range/iterator_range.hpp>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Base class for filtering the neighborhood of segment triples
    class BaseSegmentTripleRelationFilter : public Filter<Relation<const CDCSegmentTriple>> {

    public:
      /// Returns a two iterator range covering the range of possible neighboring segment triples of the given facet out of the sorted range given by the two other argumets.
      template<class ACDCSegmentTripleIterator>
      boost::iterator_range<ACDCSegmentTripleIterator>
      getPossibleNeighbors(const CDCSegmentTriple& triple,
                           const ACDCSegmentTripleIterator& itBegin,
                           const ACDCSegmentTripleIterator& itEnd)
      {

        const CDCAxialRecoSegment2D* endSegment = triple.getEndSegment();
        std::pair<ACDCSegmentTripleIterator,  ACDCSegmentTripleIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd,
            endSegment);
        return boost::iterator_range<ACDCSegmentTripleIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);

      }

      /// Legacy method
      virtual Weight isGoodNeighbor(const CDCSegmentTriple& from,
                                    const CDCSegmentTriple& to) final {

        return operator()(from, to);
      }

      /// Main filter method returning the weight of the neighborhood relation. Return NAN if relation shall be rejected.
      virtual Weight operator()(const CDCSegmentTriple& from  __attribute__((unused)),
                                const CDCSegmentTriple& to  __attribute__((unused)))
      {
        return NAN;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      virtual Weight operator()(const Relation<const CDCSegmentTriple>& relation) override final
      {
        const CDCSegmentTriple* ptrFrom(relation.first);
        const CDCSegmentTriple* ptrTo(relation.second);
        if (not ptrFrom or not ptrTo) return NAN;
        return operator()(*ptrFrom, *ptrTo);
      }


    };

  }
}

