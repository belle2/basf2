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

#include <tracking/trackFindingCDC/hough/DiscreteValue.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Phantom type tag for the discrete z0 representation
    class Z0Tag;

    /// Type for discrete phi0 values
    using DiscreteZ0 = DiscreteValue<float, Z0Tag>;

  } // end namespace TrackFindingCDC
} // end namespace Belle2
