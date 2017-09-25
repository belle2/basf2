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

#include <tracking/trackFindingCDC/filters/base/Filter.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  template <class ASeed, class AHitObject> class CKFState;
  /**
   * Base filter for detector track combination filters.
   */
  using BaseCKFCDCToSpacePointStateObjectFilter =
    TrackFindingCDC::Filter<CKFState<RecoTrack, SpacePoint>>;
}
