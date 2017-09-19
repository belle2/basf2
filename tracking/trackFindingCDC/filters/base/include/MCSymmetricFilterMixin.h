/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
