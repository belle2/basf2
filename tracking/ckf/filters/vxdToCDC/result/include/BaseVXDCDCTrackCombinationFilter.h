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
#include <tracking/ckf/states/CKFResult.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
   * Base filter for detector track combination filters.
   */
  using BaseVXDCDCTrackCombinationFilter =
    TrackFindingCDC::Filter<CKFResult<RecoTrack, TrackFindingCDC::CDCRLWireHit>>;
}
