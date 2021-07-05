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
#include <tracking/ckf/pxd/entities/CKFToPXDResult.h>

namespace Belle2 {
  /// Base filter for CKF PXD results (on overlap check)
  using BasePXDResultFilter = TrackFindingCDC::Filter<CKFToPXDResult>;
}
