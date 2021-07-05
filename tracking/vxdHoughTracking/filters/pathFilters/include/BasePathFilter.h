/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  namespace vxdHoughTracking {
    /// Base filter for hits stored in the VXDHoughState
    using BasePathFilter =
      TrackFindingCDC::Filter<std::pair<const std::vector<TrackFindingCDC::WithWeight<const VXDHoughState*>>, VXDHoughState*>>;
  }
}
