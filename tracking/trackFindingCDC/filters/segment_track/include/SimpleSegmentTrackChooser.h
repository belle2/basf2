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

#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrackChooser.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the construction of good segment - track pairs
    class SimpleSegmentTrackChooser : public BaseSegmentTrackChooser {

    private:
      /// Type of the super class
      typedef BaseSegmentTrackChooser Super;

    public:
      /// Constructor
      SimpleSegmentTrackChooser() : Super() { }

    public:
      virtual CellWeight operator()(const std::pair<const CDCRecoSegment2D*, const CDCTrack*>& testPair) IF_NOT_CINT(override final);

    private:

    };
  }
}
