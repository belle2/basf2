/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segment_track_chooser/BaseSegmentTrackChooser.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class AllSegmentTrackChooser : public BaseSegmentTrackChooser {
      /** Accept all segment track combinations */
      virtual CellWeight operator()(const std::pair<const CDCRecoSegment2D*, const CDCTrack*>&)
      {
        return 1;
      }

    };
  }
}
