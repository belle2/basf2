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
    namespace NSZParameterIndices {

      /// Enumeration to address the individual sz parameters in a vector or matrix
      enum ESZParameter {
        /// Constant to address the tanLambda in a vector or matrix assoziated with sz parameters
        c_TanL = 0,

        /// Constant to address the z reference point in a vector or matrix assoziated with sz
        /// parameters
        c_Z0 = 1,

        /// Constant for the total number of indices.
        c_N = 2,
      };
    }

    /// Importing the enumeration for the namespace but not the constants.
    using ESZParameter = NSZParameterIndices::ESZParameter;

    /// Utiliy functions and types related to the sz plane parameters
    struct SZUtil : UncertainParametersUtil<SZUtil, ESZParameter> {

      /// Getter for the sign change of the sz parameters on reversal of the trajectory
      static ParameterVector reversalSigns()
      {
        ParameterVector result;
        result << -1, 1;
        return result;
      }
    };

    /// Vector of the sz parameters
    using SZParameters = SZUtil::ParameterVector;

    /// Covariance matrix of the sz parameters
    using SZCovariance = SZUtil::CovarianceMatrix;

    /// Precision matrix of the sz parameters
    using SZPrecision = SZUtil::PrecisionMatrix;

    /// Jacobian matrix for the sz parameters
    using SZJacobian = SZUtil::JacobianMatrix;

  } // namespace TrackFindingCDC
} // namespace Belle2
