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

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>


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

      inline const CDCSegmentTriple getLowestPossibleNeighbor(const CDCSegmentTriple& triple) const
      { return CDCSegmentTriple::getLowerBound(triple.getEnd()); }

      inline
      bool
      isStillPossibleNeighbor(
        const CDCSegmentTriple& triple,
        const CDCSegmentTriple& neighborTriple,
        const CDCSegmentTriple& lowestPossibleNeighbor __attribute__((unused))
      ) const {

        return triple.getEnd() == neighborTriple.getStart();

      }

      inline Weight isGoodNeighbor(
        const CDCSegmentTriple& triple __attribute__((unused)) ,
        const CDCSegmentTriple& neighborTriple,
        const CDCSegmentTriple& lowestPossibleNeighbor __attribute__((unused))
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
