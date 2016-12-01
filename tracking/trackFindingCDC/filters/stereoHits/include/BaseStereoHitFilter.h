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
  namespace TrackFindingCDC {
    class CDCRecoHit3D;
    class CDCTrack;
    /// Base filter for stereo hit - track relations.
    using BaseStereoHitFilter = Filter<std::pair<const CDCRecoHit3D*, const CDCTrack*>>;
  }
}
