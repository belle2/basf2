/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>
#include <tracking/trackingUtilities/utilities/Functional.h>

namespace Belle2 {
  namespace TrackingUtilities {

    /// Comparer functor comparing the wire hit of two given hits
    using HitComperator = LessOf<MayIndirectTo<GetWireHit>>;
  }
}
