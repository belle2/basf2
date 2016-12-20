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
#include <tracking/trackFindingCDC/collectors/adders/AdderInterface.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRLWireHit;

    /** Class to add the matched hits to the track and set the taken flag correctly. */
    class StereoHitTrackAdder : public AdderInterface<CDCTrack, CDCRLWireHit> {
    private:
      /** Add the matched hits to the track and set the taken flag correctly. We ignore the weight completely here. */
      void add(CDCTrack& track, const CDCRLWireHit& hit, Weight weight) override;
    };
  }
}
