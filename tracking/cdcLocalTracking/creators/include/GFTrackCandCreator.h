/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GFTRACKCANDCREATOR_H_
#define GFTRACKCANDCREATOR_H_

#include <tracking/cdcLocalTracking/eventdata/tracks/CDCTrack.h>

#include <framework/datastore/StoreArray.h>

#include "genfit/TrackCand.h"

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing construction of genfit track candidates from reconstructed tracks
    class GFTrackCandCreator {

    public:

      /** Constructor. */
      GFTrackCandCreator();

      /** Destructor.*/
      ~GFTrackCandCreator() {;}

      /// Translates all CDCTracks to genfit tracks
      void create(const std::vector<CDCTrack>& cdcTracks,
                  StoreArray<genfit::TrackCand>& gfTrackCands) const;

    private:
      /// Copies the hit content of the CDCTrack to the genfit track and sets its initial parameters.
      void create(const CDCTrack& track, genfit::TrackCand& gfTrackCand) const;

      /// Copies the hit content of the CDCTrack to the genfit track.
      inline void appendHits(const CDCTrack& track, genfit::TrackCand& gfTrackCand) const;

    private:

    }; // end class WireHitCreator


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //GFTRACKCANDCREATOR_H_
