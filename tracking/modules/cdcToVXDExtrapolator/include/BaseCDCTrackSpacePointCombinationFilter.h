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
#include <tracking/modules/cdcToVXDExtrapolator/CKFCDCToVXDResultObject.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /**
   * Base filter for detector track combination filters. Please note that for implementation reasons
   * the base object includes poitners to pointers.
   */
  using BaseCDCTrackSpacePointCombinationFilter =
    TrackFindingCDC::Filter<CKFCDCToVXDStateObject>;
}
