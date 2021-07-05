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

#include <tracking/trackFindingCDC/filters/wireHit/BaseWireHitFilter.h>
#include <tracking/trackFindingCDC/filters/wireHit/WireHitFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Guard to prevent repeated instantiations
    extern template class Chooseable<BaseWireHitFilter>;
    extern template class ChooseableFilter<WireHitFilterFactory>;
    using ChooseableWireHitFilter = ChooseableFilter<WireHitFilterFactory>;
  }
}
