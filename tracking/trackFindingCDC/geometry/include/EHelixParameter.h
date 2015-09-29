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

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Namespace to hide the contained enum constants
    namespace NHelixParameter {

      /// Enumeration to address the individual helix parameters in a vector or matrix
      enum EHelixParameter {
        /// Constant to address the curvature in a vector or matrix assoziated with perigee parameters.
        c_Curv = 0,

        /// Constant to address the azimuth angle of the direction of flight at the perigee in a vector or matrix assoziated with perigee parameters.
        c_Phi0 = 1,

        /// Constant to address the impact parameter in a vector or matrix assoziated with perigee parameters.
        c_I = 2,

        /// Constant to address the tanLambda in a vector or matrix assoziated with helix parameters
        c_TanL = 3,

        /// Constant to address the z reference point in a vector or matrix assoziated with helix parameters
        c_Z0 = 4,
      };
    }

    /// Importing the enumeration for the namespace but not the constants.
    using EHelixParameter = NHelixParameter::EHelixParameter;

  } // namespace TrackFindingCDC

} // namespace Belle2

