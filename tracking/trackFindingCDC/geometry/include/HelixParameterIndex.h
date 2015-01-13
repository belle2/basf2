/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef HELIXPARAMETERINDEX_H
#define HELIXPARAMETERINDEX_H

#include "PerigeeParameterIndex.h"

namespace Belle2 {

  namespace CDCLocalTracking {

    /// Helper indices for meaningfull access in a vector or matrix assoziated with helix parameters.
    typedef PerigeeParameterIndex HelixParameterIndex;

    // Implicit from PerigeeParameterIndex
    //const HelixParameterIndex iCurv = 0;
    //const HelixParameterIndex iPhi0 = 1;
    //const HelixParameterIndex iI = 2;

    /// Constant to address the szSlope in a vector or matrix assoziated with helix parameters
    const HelixParameterIndex iSZ = 3;

    /// Constant to address the z reference point in a vector or matrix assoziated with helix parameters
    const HelixParameterIndex iZ0 = 4;

  } // namespace CDCLocalTracking

} // namespace Belle2

#endif // HELIXPARAMETERINDEX_H
