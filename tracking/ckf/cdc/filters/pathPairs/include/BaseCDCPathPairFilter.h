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

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <utility>

namespace Belle2 {
  /// Base filter for CKF CDC paths
  using BaseCDCPathPairFilter = TrackFindingCDC::Filter<std::pair<const CDCCKFPath*, const CDCCKFPath*>>;
}
