/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

namespace Belle2 {
  /// Base filter for CKF PXD states
  using BasePXDStateFilter =
    TrackFindingCDC::Filter<std::pair<const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>, CKFToPXDState*>>;
}
