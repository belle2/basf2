/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /** Comparer function (according to std::less) for two RLTaggedWireHits.
     * The reason for this class is a bit non-obvious. Imagine the following scenario:
     * We try to match RLTaggedWireHits to Tracks. One RLWireHit can be matched to a track. The
     * same WireHit, but with a different RLInformation, can be matched to another track.
     * If we compare those two hits without further knowledge, we would end up in two hits. Both will be added
     * to different tracks resulting in adding the same wire twice.
     */
    class StereoHitLessOperator {
    public:
      bool operator()(const CDCRLTaggedWireHit* lhs, const CDCRLTaggedWireHit* rhs) const
      {
        return lhs->getWireHit() < rhs->getWireHit();
      }
    };
  }
}
