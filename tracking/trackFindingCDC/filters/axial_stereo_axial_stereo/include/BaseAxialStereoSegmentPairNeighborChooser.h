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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialStereoSegmentPair.h>

#include <tracking/trackFindingCDC/algorithms/NeighborWeight.h>
#include <tracking/trackFindingCDC/algorithms/Relation.h>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>
#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {
    ///Base class for filtering the neighborhood of axial stereo segment pairs

    template<>
    class Filter<Relation<CDCAxialStereoSegmentPair>>  :
                                                     public FilterBase<Relation<CDCAxialStereoSegmentPair>> {

    public:
      /// Returns a two iterator range covering the range of possible neighboring axial stereo segment pairs of the given axial stereo segment pair out of the sorted range given by the two other argumets.
      template<class CDCAxialStereoSegmentPairIterator>
      boost::iterator_range<CDCAxialStereoSegmentPairIterator>
      getPossibleNeighbors(const CDCAxialStereoSegmentPair& axialStereoSegmentPair,
                           const CDCAxialStereoSegmentPairIterator& itBegin,
                           const CDCAxialStereoSegmentPairIterator& itEnd) const
      {

        const CDCRecoSegment2D* ptrEndSegment = axialStereoSegmentPair.getEndSegment();
        if (not ptrEndSegment) boost::iterator_range<CDCAxialStereoSegmentPairIterator>(itEnd, itEnd);

        std::pair<CDCAxialStereoSegmentPairIterator, CDCAxialStereoSegmentPairIterator> itPairPossibleNeighbors = std::equal_range(itBegin,
            itEnd, ptrEndSegment);
        return boost::iterator_range<CDCAxialStereoSegmentPairIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);
      }

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NOT_A_NEIGHBOR to reject all axial stereo segment pair neighbors.
       */
      virtual NeighborWeight operator()(const CDCAxialStereoSegmentPair& /* from */,
                                        const CDCAxialStereoSegmentPair& /* to */)
      {
        return NOT_A_NEIGHBOR;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      virtual CellWeight operator()(const Relation<CDCAxialStereoSegmentPair>& relation) override final
      {
        const CDCAxialStereoSegmentPair* ptrFrom = relation.first;
        const CDCAxialStereoSegmentPair* ptrTo = relation.second;
        if (not ptrFrom or not ptrTo) return NOT_A_NEIGHBOR;
        return operator()(*ptrFrom, *ptrTo);
      }

      /// Legacy method
      virtual NeighborWeight isGoodNeighbor(const CDCAxialStereoSegmentPair& from,
                                            const CDCAxialStereoSegmentPair& to)
      {
        return operator()(from, to);
      }
    }; // end class

    /// Alias for the base class for filtering the neighborhood of axial stereo segment pairs.
    typedef Filter<Relation<CDCAxialStereoSegmentPair>>  BaseAxialStereoSegmentPairNeighborChooser;

  } //end namespace TrackFindingCDC
} //end namespace Belle2
