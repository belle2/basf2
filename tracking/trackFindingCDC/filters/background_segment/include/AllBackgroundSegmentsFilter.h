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

#include <tracking/trackFindingCDC/filters/background_segment/BaseBackgroundSegmentsFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class AllBackgroundSegmentsFilter : public BaseBackgroundSegmentsFilter {
      /** Accept all segment track combinations */
      virtual CellWeight operator()(const CDCRecoSegment2D&)
      {
        return 1;
      }

    };
  }
}
