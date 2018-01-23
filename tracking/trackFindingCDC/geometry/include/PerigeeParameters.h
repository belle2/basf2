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

    // Guard to prevent repeated template symbol emission
    struct PerigeeUtil;
    extern template struct UncertainParametersUtil<PerigeeUtil, EPerigeeParameter>;

    /// Utility struct for functions and types related to the perigee parameters.
    struct PerigeeUtil : UncertainParametersUtil<PerigeeUtil, EPerigeeParameter> {

      /// Getter for the signs which have to be applied to reverse the traversal direction
      static ParameterVector reversalSigns()
      {
        return ParameterVector({ -1.0, 1.0, -1.0});
      }

      /**
       *  Calculates the weighted average between two perigee parameter sets
       *  with their respective covariance matrix.
       *
       *  Returns the chi2 value of the average.
       */
      static double average(const PerigeeUtil::ParameterVector& fromPar,
                            const PerigeeUtil::CovarianceMatrix& fromCov,
                            const PerigeeUtil::ParameterVector& toPar,
                            const PerigeeUtil::CovarianceMatrix& toCov,
                            PerigeeUtil::ParameterVector& avgPar,
                            PerigeeUtil::CovarianceMatrix& avgCov);
    };

    /// Vector of the perigee parameters
    using PerigeeParameters = PerigeeUtil::ParameterVector;

    /// Covariance matrix of the perigee parameters
    using PerigeeCovariance = PerigeeUtil::CovarianceMatrix;

    /// Precision matrix of the perigee parameters
    using PerigeePrecision = PerigeeUtil::PrecisionMatrix;

    /// Jacobian matrix for the perigee parameters
    using PerigeeJacobian = PerigeeUtil::JacobianMatrix;

  }

}
