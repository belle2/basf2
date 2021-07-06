/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
