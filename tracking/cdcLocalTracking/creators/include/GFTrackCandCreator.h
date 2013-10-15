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

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <framework/datastore/StoreArray.h>

#include "GFTrackCand.h"

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing construction of genfit track candidates from reconstructed tracks
    class GFTrackCandCreator {

    public:

      /** Constructor. */
      GFTrackCandCreator();

      /** Destructor.*/
      ~GFTrackCandCreator() {;}

      void create(const std::vector<CDCTrack>& cdcTracks,
                  StoreArray<GFTrackCand>& gfTrackCands) const;

    private:
      void create(const CDCTrack& track, GFTrackCand& gfTrackCand) const;


      inline void appendHits(const CDCTrack& track, GFTrackCand& gfTrackCand) const;

    private:


    }; // end class WireHitCreator


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //GFTRACKCANDCREATOR_H_
