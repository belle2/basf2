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
    class CDCSegment2D;

    /** Class to add the matched segments to the track and set the taken flag correctly. */
    class SegmentTrackAdder : public AdderInterface<CDCTrack, CDCSegment2D> {
    private:
      /** Add the matched segment to the track and set the taken flag correctly. We ignore the weight completely here. */
      void add(CDCTrack& track, const CDCSegment2D& segment, Weight weight) override;
    };
  }
}
