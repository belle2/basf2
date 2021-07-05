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

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Comparer functor comparing the wire hit of two given hits
    using HitComperator = LessOf<MayIndirectTo<GetWireHit>>;
  }
}
