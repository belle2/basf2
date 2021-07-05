/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
