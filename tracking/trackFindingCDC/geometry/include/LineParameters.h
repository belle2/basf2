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

#include <tracking/trackFindingCDC/geometry/UncertainParameters.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Namespace to hide the contained enum constants
    namespace NLineParameterIndices {

      /// Enumeration to address the individual helix parameters in a vector or matrix
      enum ELineParameter {
        /// Constant to address the azimuth angle of the direction of flight at the perigee in a
        /// vector or matrix assoziated with perigee parameters.
        c_Phi0 = 0,

        /// Constant to address the impact parameter in a vector or matrix assoziated with perigee
        /// parameters.
        c_I = 1,

        /// Constant for the total number of indices.
        c_N = 2,
      };
    }

    /// Importing the enumeration for the namespace but not the constants.
    using ELineParameter = NLineParameterIndices::ELineParameter;

    /// Utiliy functions and types related to the sz plane parameters
    struct LineUtil : UncertainParametersUtil<LineUtil, ELineParameter> {

      /// Getter for the sign change of the line parameters on reversal of the trajectory
      static ParameterVector reversalSigns()
      {
        return ParameterVector{1, -1};
      }
    };

    /// Vector of the line parameters
    using LineParameters = LineUtil::ParameterVector;

    /// Covariance matrix of the line parameters
    using LineCovariance = LineUtil::CovarianceMatrix;

    /// Precision matrix of the line parameters
    using LinePrecision = LineUtil::PrecisionMatrix;

    /// Jacobian matrix for the line parameters
    using LineJacobian = LineUtil::JacobianMatrix;

  } // namespace TrackFindingCDC
} // namespace Belle2
