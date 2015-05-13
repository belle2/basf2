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

#include <tracking/trackFindingCDC/filters/segment_track/MCSegmentTrainFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the construction of good segment - track pairs
    typedef MCSegmentTrainFilter MCSegmentTrackFilter;
  }
}
