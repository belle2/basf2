/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SINGLESEGMENTTRACKCREATOR_H_
#define SINGLESEGMENTTRACKCREATOR_H_

#include <vector>
#include <tracking/cdcLocalTracking/eventdata/tracks/CDCTrack.h>
#include "TrackCreator.h"

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Creator to append left over segments from the cellular automaton as tracks
    /** The cellular automaton for track building is based in segment triples which means that
     *  tracks that do not traverse three superlayers are not picked up.
     *  This step seeks to clean up the left over segments.
     *  The implementation is not very mature or clever by any means.
     *  For now we are just taking the segments in the first superlayer that have not been used yet.
     *  Preliminary test show that the total construction efficiency over a sample of ( ) increase by 6%
     *  when activating this. */
    class SingleSegmentTrackCreator {

    public:

      /// Empty constructor
      SingleSegmentTrackCreator();

      /// Empty destructor.
      virtual ~SingleSegmentTrackCreator();

      /// Augments the given segments to tracks and appends them to the given vector of tracks
      void
      append(
        const std::vector<CDCRecoSegment2D>& recoSegments,
        std::vector<CDCTrack>& tracks)
      const;

    private:
      TrackCreator m_trackCreator; ///< Instance of the track creator internally used for the appending of hits.

    }; // end class SingleSegmentTrackCreator


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SINGLESEGMENTTRACKCREATOR_H_
