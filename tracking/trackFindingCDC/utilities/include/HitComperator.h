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

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Comparer function (according to std::less) for two hits.
     */
    class HitComperator {
    public:
      /**
       * Main method of the comparator class which does the operation (according to std::less)
       * for CDCRLWireHits.
       * @param lhs: First CDCRLWireHit to compare.
       * @param rhs: Second CDCRLWireHit to compare.
       *
       * @return True, if the wire hit of the lhs is less than the wire hit of the rhs.
       */
      bool operator()(const CDCRLWireHit* lhs, const CDCRLWireHit* rhs) const
      {
        return lhs->getWireHit() < rhs->getWireHit();
      }
    };
  }
}
