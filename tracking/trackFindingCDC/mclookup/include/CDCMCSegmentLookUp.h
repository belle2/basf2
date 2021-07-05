/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
