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
    namespace NPerigeeParameterIndices {

      /// Enumeration to address the individual perigee parameters in a vector or matrix
      enum EPerigeeParameter {
        /// Constant to address the curvature in a vector or matrix assoziated with perigee
        /// parameters.
        c_Curv = 0,

        /// Constant to address the azimuth angle of the direction of flight at the perigee in a
        /// vector or matrix assoziated with perigee parameters.
        c_Phi0 = 1,

        /// Constant to address the impact parameter in a vector or matrix assoziated with perigee
        /// parameters.
        c_I = 2,

        /// Constant for the total number of indices.
        c_N = 3,
      };
    }

    /// Importing the enumeration for the namespace but not the constants.
    using EPerigeeParameter = NPerigeeParameterIndices::EPerigeeParameter;

    /// Utiliy functions and types related
    struct PerigeeUtil : UncertainParametersUtil<PerigeeUtil, EPerigeeParameter> {
      /// Getter for the indices;
      static ParameterVector reversalSigns()
      {
        ParameterVector result;
        result << -1, 1, -1;
        return result;
      }
    };

    /// Vector of the perigee parameters
    using PerigeeParameters = PerigeeUtil::ParameterVector;

    /// Covariance matrix of the perigee parameters
    using PerigeeCovariance = PerigeeUtil::CovarianceMatrix;

    /// Precision matrix of the perigee parameters
    using PerigeePrecision = PerigeeUtil::PrecisionMatrix;

    /// Jacobian matrix for the perigee parameters
    using PerigeeJacobian = PerigeeUtil::JacobianMatrix;

  } // namespace TrackFindingCDC

} // namespace Belle2
