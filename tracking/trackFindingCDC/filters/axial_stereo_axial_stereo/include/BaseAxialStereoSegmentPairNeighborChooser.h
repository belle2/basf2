/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BASEAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H
#define BASEAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H

#include <boost/range/iterator_range.hpp>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/algorithms/NeighborWeight.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialStereoSegmentPair.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    ///Base class for filtering the neighborhood of axial stereo segment pairs
    class BaseAxialStereoSegmentPairNeighborChooser {

    public:
      /// Default constructor
      BaseAxialStereoSegmentPairNeighborChooser() {;}

      /// Empty destructor
      virtual ~BaseAxialStereoSegmentPairNeighborChooser() {;}

      /// Clears information from former events
      virtual void clear() {;}

      /// Forwards the initialize method from the module
      virtual void initialize() {;}

      /// Forwards the terminate method from the module
      virtual void terminate() {;}


      /// Returns a two iterator range covering the range of possible neighboring axial stereo segment pairs of the given axial stereo segment pair out of the sorted range given by the two other argumets.
      template<class CDCAxialStereoSegmentPairIterator>
      boost::iterator_range<CDCAxialStereoSegmentPairIterator> getPossibleNeighbors(const CDCAxialStereoSegmentPair& axialStereoSegmentPair,
          const CDCAxialStereoSegmentPairIterator& itBegin,
          const CDCAxialStereoSegmentPairIterator& itEnd) const {

        const CDCRecoSegment2D* ptrEndSegment = axialStereoSegmentPair.getEndSegment();
        if (not ptrEndSegment) boost::iterator_range<CDCAxialStereoSegmentPairIterator>(itEnd, itEnd);

        std::pair<CDCAxialStereoSegmentPairIterator, CDCAxialStereoSegmentPairIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd, ptrEndSegment);
        return boost::iterator_range<CDCAxialStereoSegmentPairIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);

      }

      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      virtual NeighborWeight isGoodNeighbor(const CDCAxialStereoSegmentPair&,
                                            const CDCAxialStereoSegmentPair&) {

        return NOT_A_NEIGHBOR;

      }
    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // BASEAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H
