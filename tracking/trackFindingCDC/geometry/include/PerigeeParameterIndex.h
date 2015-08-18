/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Helper indices for meaningfull access in a vector or matrix assoziated with perigee parameters.
    typedef Index PerigeeParameterIndex;

    /// Constant to address the curvature in a vector or matrix assoziated with perigee parameters.
    const PerigeeParameterIndex iCurv = 0;

    /// Constant to address the azimuth angle of the direction of flight at the perigee in a vector or matrix assoziated with perigee parameters.
    const PerigeeParameterIndex iPhi0 = 1;

    /// Constant to address the impact parameter in a vector or matrix assoziated with perigee parameters.
    const PerigeeParameterIndex iI = 2;

  } // namespace TrackFindingCDC

} // namespace Belle2

