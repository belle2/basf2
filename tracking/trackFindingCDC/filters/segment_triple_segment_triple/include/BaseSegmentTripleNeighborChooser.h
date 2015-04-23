/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BASESEGMENTTRIPLENEIGHBORCHOOSER_H_
#define BASESEGMENTTRIPLENEIGHBORCHOOSER_H_

#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/algorithms/NeighborWeight.h>
#include <tracking/trackFindingCDC/algorithms/Relation.h>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>
#include <boost/range/iterator_range.hpp>


namespace Belle2 {
  namespace TrackFindingCDC {

    ///Base class for filtering the neighborhood of segment triples
    template<>
    class Filter<Relation<CDCSegmentTriple>>  :
                                            public FilterBase<Relation<CDCSegmentTriple>> {

    public:
      /// Returns a two iterator range covering the range of possible neighboring segment triples of the given facet out of the sorted range given by the two other argumets.
      template<class CDCSegmentTripleIterator>
      boost::iterator_range<CDCSegmentTripleIterator>
      getPossibleNeighbors(const CDCSegmentTriple& triple,
                           const CDCSegmentTripleIterator& itBegin,
                           const CDCSegmentTripleIterator& itEnd)
      {

        const CDCAxialRecoSegment2D* endSegment = triple.getEnd();
        std::pair<CDCSegmentTripleIterator,  CDCSegmentTripleIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd,
            endSegment);
        return boost::iterator_range<CDCSegmentTripleIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);

      }

      /// Legacy method
      virtual NeighborWeight isGoodNeighbor(const CDCSegmentTriple& from,
                                            const CDCSegmentTriple& to) final {

        return operator()(from, to);
      }

      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      virtual NeighborWeight operator()(const CDCSegmentTriple& /* from */,
                                        const CDCSegmentTriple& /* to */)
      {
        return NOT_A_NEIGHBOR;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      virtual CellWeight operator()(const Relation<CDCSegmentTriple>& relation) override final
      {
        const CDCSegmentTriple* ptrFrom = relation.first;
        const CDCSegmentTriple* ptrTo = relation.second;
        if (not ptrFrom or not ptrTo) return NOT_A_NEIGHBOR;
        return operator()(*ptrFrom, *ptrTo);
      }


    }; // end class

    /// Alias for the base class for filtering the neighborhood of segment triples.
    typedef Filter<Relation<CDCSegmentTriple>>  BaseSegmentTripleNeighborChooser;

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //BASESEGMENTTRIPLENEIGHBORCHOOSER_H_
