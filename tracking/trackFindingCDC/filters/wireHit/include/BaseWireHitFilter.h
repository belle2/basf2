/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Cyrille Praz                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Guard to prevent repeated instantiations
    extern template class Filter<CDCWireHit>;

    /// Base class for wire hit filters
    using BaseWireHitFilter = Filter<CDCWireHit>;
  }
}
