/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <genfit/TrackCand.h>
#include <genfit/WireTrackCandHit.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    struct GenfitUtil {

      /**
       *  Translates a range of hits and inserts them in the genfit::TrackCand
       *
       *  @param rlWireHitHolders  A range of hits, which elements support ->getRLWireHit()
       *                           to access a wire hit including a right left passage hypotheses.
       *  @param[out] gfTrackCand  Genfit track candidate to be filled
       */
      template<class ARLHitHolderRange>
      static void fill(genfit::TrackCand& gfTrackCand, const ARLHitHolderRange& rlWireHitHolders)
      {
        int sortingParameter = -1;
        for (const auto& rlWireHitHolder : rlWireHitHolders) {
          ++sortingParameter;

          const CDCRLTaggedWireHit rlWireHit = rlWireHitHolder->getRLWireHit();
          const CDCWireHit& wireHit = rlWireHit.getWireHit();
          const CDCWire& wire = rlWireHit.getWire();

          // the hit id correspondes to the index in the TClonesArray
          // this is stored in the wirehit the recohit is based on
          unsigned int storeIHit = wireHit.getStoreIHit();

          // the plain id serves to mark competition between two or more hits
          // use the wire id here which is unique for all the hits in the track
          // but it may also serve the fact that a track can only be responsable for a single hit on each wire
          // double hits should correspond to multiple tracks
          unsigned int planeId = wire.getEWire();

          // Right left ambiguity resolution
          ERightLeft rlInfo = rlWireHit.getRLInfo();

          // Note:  rlInfo < 0 means ERightLeft::c_Left,   rlInfo > 0 means ERightLeft::c_Right,  which is the same as in Genfit
          signed char genfitLeftRight = rlInfo;

          //do not delete! the genfit::TrackCand has ownership
          genfit::WireTrackCandHit* cdcTrackCandHit =
            new genfit::WireTrackCandHit(Const::CDC,
                                         storeIHit,
                                         planeId,
                                         sortingParameter,
                                         genfitLeftRight);
          gfTrackCand.addHit(cdcTrackCandHit);

        }
      }
    }; // struct GenfitUtil
  } // namespace TrackFindingCDC
} // namespace Belle2
