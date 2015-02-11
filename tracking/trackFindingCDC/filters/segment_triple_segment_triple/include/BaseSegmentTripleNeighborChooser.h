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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/algorithms/NeighborWeight.h>

#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {

    ///Base class for filtering the neighborhood of segment triples
    /** Base implementation providing the getLowestPossibleNeighbor and isStillPossibleNeighbor method
     *  enforcing the indentity of end segment of this triple and the start segment of the neighbor
     *  Besides that it accepts all segment triple. The isGoodNeighbor method has to be made more sophisticated by a subclass.
     */
    class BaseSegmentTripleNeighborChooser {


    public:
      /// Default constructor
      BaseSegmentTripleNeighborChooser()
      {;}

      /// Empty destructor
      virtual ~BaseSegmentTripleNeighborChooser() {;}

      /// Clears stored information for a former event
      virtual void clear() {;}

      /// Forwards the initialize method from the module
      virtual void initialize() {;}

      /// Forwards the terminate method from the module
      virtual void terminate() {;}


      /// Returns a two iterator range covering the range of possible neighboring segment triples of the given facet out of the sorted range given by the two other argumets.
      template<class CDCSegmentTripleIterator>
      boost::iterator_range<CDCSegmentTripleIterator>
      getPossibleNeighbors(const CDCSegmentTriple& triple,
                           const CDCSegmentTripleIterator& itBegin,
                           const CDCSegmentTripleIterator& itEnd)  {

        const CDCAxialRecoSegment2D* endSegment = triple.getEnd();
        std::pair<CDCSegmentTripleIterator,  CDCSegmentTripleIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd, endSegment);
        return boost::iterator_range<CDCSegmentTripleIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);

      }

      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      virtual NeighborWeight isGoodNeighbor(const CDCSegmentTriple&,
                                            const CDCSegmentTriple&) {

        return NOT_A_NEIGHBOR;
      }
    }; // end class


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //BASESEGMENTTRIPLENEIGHBORCHOOSER_H_
