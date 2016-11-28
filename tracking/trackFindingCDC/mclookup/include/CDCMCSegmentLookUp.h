/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Legacy for python : interface to obtain the two dimensional segment mc lookup
    class CDCMCSegmentLookUp {

    public:
      /// Getter for the singletone instance
      static const CDCMCSegment2DLookUp& getInstance();
    };
  }
}
