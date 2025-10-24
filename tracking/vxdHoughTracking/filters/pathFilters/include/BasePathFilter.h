/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  namespace vxdHoughTracking {
    class VXDHoughState;
    /// Base filter for hits stored in the VXDHoughState
    using BasePathFilter =
      TrackFindingCDC::Filter<std::pair<const std::vector<TrackFindingCDC::WithWeight<const VXDHoughState*>>, VXDHoughState*>>;
  }
}
