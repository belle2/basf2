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

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  class CDCHit;

  namespace TrackFindingCDC {

    /// Structure to summarize utility function to output a list of hits into a genfit::TrackCand (deprecated) or a RecoTrack.
    struct RecoTrackUtil {

      /**
       *  Translates a range of hits and inserts them in the reco track.
       *
       *  @param rlWireHitHolders  A range of hits, which elements support ->getRLWireHit()
       *                           to access a wire hit including a right left passage hypotheses.
       *  @param[out] recoTrack    RecoTrack to be filled.
       */
      template<class ARLHitHolderRange>
      static void fill(const ARLHitHolderRange& rlWireHitHolders, RecoTrack& recoTrack)
      {
        int sortingParameter = -1;
        for (const auto& rlWireHitHolder : rlWireHitHolders) {
          ++sortingParameter;

          const CDCRLWireHit rlWireHit = rlWireHitHolder.getRLWireHit();
          const CDCWireHit& wireHit = rlWireHit.getWireHit();
          const CDCHit* cdcHit = wireHit.getHit();

          // Right left ambiguity resolution
          ERightLeft rlInfo = rlWireHit.getRLInfo();

          if (rlInfo == ERightLeft::c_Left) {
            recoTrack.addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_left);
          } else if (rlInfo == ERightLeft::c_Right) {
            recoTrack.addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_right);
          } else if (rlInfo == ERightLeft::c_Invalid) {
            recoTrack.addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_invalidRightLeftInformation);
          } else {
            recoTrack.addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_undefinedRightLeftInformation);
          }
        }
      }
    };
  }
}
