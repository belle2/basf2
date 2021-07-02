/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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