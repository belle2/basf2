/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLEAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H
#define SIMPLEAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H

#include "BaseAxialStereoSegmentPairNeighborChooser.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class filtering the neighborhood of segment triples based on simple criterions.
    class SimpleAxialStereoSegmentPairNeighborChooser : public BaseAxialStereoSegmentPairNeighborChooser {

    public:
      /** Constructor. */
      SimpleAxialStereoSegmentPairNeighborChooser()
      {;}

      /** Destructor.*/
      ~SimpleAxialStereoSegmentPairNeighborChooser() {;}

      inline Weight isGoodNeighbor(
        const CDCAxialStereoSegmentPair& pair __attribute__((unused)) ,
        const CDCAxialStereoSegmentPair& neighborPair
      ) const {

        // Just let all found neighors pass since we have the same start -> end segment
        // and let the cellular automaton figure auto which is longest

        // can of course be adjusted by comparing the z components between
        // triple and neighbor triple

        // neighbor weight is a penalty for the overlap of the segments since we would
        // count it to times
        // could also be a better measure of fit quality

        return  -neighborPair.getStartSegment()->size();

      }

    private:

    }; // end class

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif // SIMPLEAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H
