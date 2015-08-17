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
#include <tracking/trackFindingCDC/hough/ContinuousValue.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Phantom type tag for the discrete tan lambda representation
    class TanLTag;

    /// Type for discrete tan lambda values
    using DiscreteTanL = DiscreteValue<float, TanLTag>;

    /// Type to have continuous tan lambda values
    using ContinuousTanL = ContinuousValue<double, TanLTag>;

  } // end namespace TrackFindingCDC
} // end namespace Belle2
