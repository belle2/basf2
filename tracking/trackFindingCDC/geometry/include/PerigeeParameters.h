/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
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
        /// Constant to address the curvature
        c_Curv = 0,

        /// Constant to address the azimuth angle of the direction of flight at the perigee
        c_Phi0 = 1,

        /// Constant to address the impact parameter
        c_I = 2,

        /// Constant for the total number of indices.
        c_N = 3,
      };
    }

    /// Importing the enumeration for the namespace but not the constants.
    using EPerigeeParameter = NPerigeeParameterIndices::EPerigeeParameter;

    /// Utility struct for functions and types related to the perigee parameters.
    struct PerigeeUtil : UncertainParametersUtil<PerigeeUtil, EPerigeeParameter> {

      /// Getter for the signs which have to be applied to reverse the traversal direction
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
