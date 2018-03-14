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
    namespace NSZParameterIndices {

      /// Enumeration to address the individual sz parameters in a vector or matrix
      enum ESZParameter {
        /// Constant to address the tan lambda dip out of the xy plane
        c_TanL = 0,

        /// Constant to address the z start position
        c_Z0 = 1,

        /// Constant for the total number of indices.
        c_N = 2,
      };
    }

    /// Importing the enumeration for the namespace but not the constants.
    using ESZParameter = NSZParameterIndices::ESZParameter;

    // Guard to prevent repeated template symbol emission
    struct SZUtil;
    extern template struct UncertainParametersUtil<SZUtil, ESZParameter>;

    /// Utility struct for functions and types related to the sz plane parameters.
    struct SZUtil : UncertainParametersUtil<SZUtil, ESZParameter> {

      /// Getter for the signs which have to be applied to reverse the parameters
      static ParameterVector reversalSigns()
      {
        return  ParameterVector({ -1.0, 1.0});
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

  }

}
