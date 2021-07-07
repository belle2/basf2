/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

namespace Belle2 {
  namespace vxdHoughTracking {
    /// Base filter for hits stored in the HitData
    using BaseTrackletFilter = TrackFindingCDC::Filter<SpacePointTrackCand>;
  }
}
