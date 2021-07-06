/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// This header file is deprecated
// Instead use one of the following headers depending on the *minimal* needs of your use.
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.icc.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Mixin for filters that use Monte Carlo information.
    template <class AFilter>
    using MCSymmetricFilterMixin = MCSymmetric<AFilter>;

  }
}
