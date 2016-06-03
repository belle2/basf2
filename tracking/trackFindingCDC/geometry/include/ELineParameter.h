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
    namespace NLineParameter {

      /// Enumeration to address the individual helix parameters in a vector or matrix
      enum ELineParameter {
        /// Constant to address the azimuth angle of the direction of flight at the perigee in a vector or matrix assoziated with perigee parameters.
        c_Phi0 = 1,

        /// Constant to address the impact parameter in a vector or matrix assoziated with perigee parameters.
        c_I = 2,
      };
    }

    /// Importing the enumeration for the namespace but not the constants.
    using ELineParameter = NLineParameter::ELineParameter;

  } // namespace TrackFindingCDC
} // namespace Belle2
