/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCSEGMENTTRIPLENEIGHBORCHOOSER_H_
#define MCSEGMENTTRIPLENEIGHBORCHOOSER_H_

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include "BaseSegmentTripleNeighborChooser.h"

namespace Belle2 {
  namespace CDCLocalTracking {
    ///Class filtering the neighborhood of segment triples with monte carlo information
    class MCSegmentTripleNeighborChooser: public BaseSegmentTripleNeighborChooser {

    public:
      /** Constructor. */
      MCSegmentTripleNeighborChooser()
      {;}

      /** Destructor.*/
      ~MCSegmentTripleNeighborChooser() {;}


      inline Weight isGoodNeighbor(
        const CDCSegmentTriple& triple __attribute__((unused)) ,
        const CDCSegmentTriple& neighborTriple
      ) const {

        // Just let all found neighors pass since we have the same start -> end triple
        // and let the cellular automaton figure auto which is the best

        // can of course be adjusted by comparing the z components between
        // triple and neighbor triple

        // neighbor weight is a penalty for the overlap of the segments since we would
        // count it to times
        // could also be a better measure of fit quality

        return - neighborTriple.getStart()->size();

      }

    private:

    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //MCSEGMENTTRIPLENEIGHBORCHOOSER_H_
