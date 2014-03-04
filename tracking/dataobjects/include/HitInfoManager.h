/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <tracking/dataobjects/ETrackFinderIDCDC.h>
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  /** This class should know, which hits are taken by a track.
   *
   *  And possibly as well by which finder the hit was assigned.
   *  However, the details are not yet fully decided.
   */
  class HitInfoManager : public RelationsObject {
  public:
    HitInfoManager() {}
    void addCDCHit(unsigned short cdcHitIndex,
                   unsigned short usedInTrack,
                   ETrackFinderIDCDC trackfinderID);
  private:
    //CDCHit Management
  };

}
