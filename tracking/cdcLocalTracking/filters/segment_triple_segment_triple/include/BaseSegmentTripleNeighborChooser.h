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

#include <boost/range/iterator_range.hpp>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/NeighborWeight.h>

#include <tracking/cdcLocalTracking/eventdata/tracks/CDCSegmentTriple.h>


namespace Belle2 {
  namespace CDCLocalTracking {

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
      ~BaseSegmentTripleNeighborChooser() {;}

      /// Clears stored information for a former event
      inline void clear() const {/*nothing to remember*/;}

      /// Forwards the initialize method from the module
      void initialize() {;}

      /// Forwards the terminate method from the module
      void terminate() {;}


      /// Returns a two iterator range covering the range of possible neighboring segment triples of the given facet out of the sorted range given by the two other argumets.
      template<class CDCSegmentTripleIterator>
      boost::iterator_range<CDCSegmentTripleIterator>
      getPossibleNeighbors(
        const CDCSegmentTriple& triple,
        const CDCSegmentTripleIterator& itBegin,
        const CDCSegmentTripleIterator& itEnd
      ) const {

        const CDCAxialRecoSegment2D* endSegment = triple.getEnd();
        std::pair<CDCSegmentTripleIterator,  CDCSegmentTripleIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd, endSegment);
        return boost::iterator_range<CDCSegmentTripleIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);

      }

      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      inline Weight isGoodNeighbor(
        const CDCSegmentTriple& triple __attribute__((unused)) ,
        const CDCSegmentTriple& neighborTriple
      ) const {

        // Just let all found neighors pass for the base implementation
        // with the default weight
        return  -neighborTriple.getStart()->size();

      }

    private:

    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //BASESEGMENTTRIPLENEIGHBORCHOOSER_H_
