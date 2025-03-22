/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/trackingUtilities/collectors/adders/AdderInterface.h>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
  }
  namespace TrackFindingCDC {
    class CDCRLWireHit;

    /** Class to add the matched hits to the track and set the taken flag correctly. */
    class StereoHitTrackAdder : public TrackingUtilities::AdderInterface<TrackingUtilities::CDCTrack, CDCRLWireHit> {
    private:
      /** Add the matched hits to the track and set the taken flag correctly. We ignore the weight completely here. */
      void add(TrackingUtilities::CDCTrack& track, const CDCRLWireHit& hit, TrackingUtilities::Weight weight) override;
    };
  }
}
