/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKORIENTATOR_H_
#define TRACKORIENTATOR_H_

#include <vector>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Algorithm fixing the track orientation if we can figure that the track point into the wrong direction
    /** Since the tracks are produced from the cellular automaton in no specific direction
     *  we have to fix the orientation by hand somehow. The algorithm is at no point to be called clever
     *  we just take the end points and check which on is more to the outside.
     *  Best would be to do the orientation in combination with figuring out the event decay tree of tracks */
    class TrackOrientator {

    public:

      /// Empty constructor
      TrackOrientator();

      /// Empty destructor.
      virtual ~TrackOrientator();

      /// Set the orientation marker of each track to FORWARD or BACKWARD. Currently trivially implemented as going outward.
      void markOrientation(std::vector<CDCTrack>& tracks) const;

    private:



    }; // end class TrackOrientator


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //TRACKORIENTATOR_H_
