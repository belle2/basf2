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

#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>
#include <tracking/trackFindingCDC/geometry/SZParameters.h>

#include <tracking/trackFindingCDC/geometry/UncertainParameters.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Namespace to hide the contained enum constants
    namespace NHelixParameterIndices {

      /// Enumeration to address the individual helix parameters in a vector or matrix
      enum EHelixParameter {
        /// Constant to address the curvature in the xy plane
        c_Curv = 0,

        /// Constant to address the azimuth angle of the direction of flight
        c_Phi0 = 1,

        /// Constant to address the impact parameter
        c_I = 2,

        /// Constant to address the tan lambda dip out of the xy plane
        c_TanL = 3,

        /// Constant to address the z start position
        c_Z0 = 4,

        /// Constant for the total number of indices.
        c_N = 5,
      };
    }

    /// Importing the enumeration for the namespace but not the constants.
    using EHelixParameter = NHelixParameterIndices::EHelixParameter;

    /// Utility struct for functions and types related to the helix parameters.
    struct HelixUtil : UncertainParametersUtil<HelixUtil, EHelixParameter> {

      /// Getter for the signs which have to be applied to reverse the parameters
      static ParameterVector reversalSigns()
      {
        ParameterVector result;
        result << -1, 1, -1, -1, 1;
        return result;
      }

      /// Get helix parameters related to the xy space
      static PerigeeUtil::ParameterVector getPerigeeParameters(const ParameterVector& helixPar)
      {
        using namespace NHelixParameterIndices;
        return getSubParameterVector<PerigeeUtil::ParameterVector, c_Curv>(helixPar);
      }

      /// Get sz parameters
      static SZUtil::ParameterVector getSZParameters(const ParameterVector& helixPar)
      {
        using namespace NHelixParameterIndices;
        return getSubParameterVector<SZUtil::ParameterVector, c_TanL>(helixPar);
      }

      /// Combine parameters from the xy space and the sz space
      static ParameterVector stack(const PerigeeUtil::ParameterVector& perigeePar,
                                   const SZUtil::ParameterVector& szPar)
      {
        ParameterVector result = ParameterVectorUtil::stack(perigeePar, szPar);
        return result;
      }

      /// Get perigee covariance matrix related to the xy space
      static PerigeeUtil::CovarianceMatrix getPerigeeCovariance(const CovarianceMatrix& helixCov)
      {
        using namespace NHelixParameterIndices;
        return getSubCovarianceMatrix<PerigeeUtil::CovarianceMatrix, c_Curv>(helixCov);
      }

      /// Get sz covariance matrix
      static SZUtil::CovarianceMatrix getSZCovariance(const CovarianceMatrix& helixCov)
      {
        using namespace NHelixParameterIndices;
        return getSubCovarianceMatrix<SZUtil::CovarianceMatrix, c_TanL>(helixCov);
      }

      /// Combine covariance matrices from the xy space and the sz space in their respective blocks
      static CovarianceMatrix stackBlocks(const PerigeeUtil::CovarianceMatrix& perigeeCov,
                                          const SZUtil::CovarianceMatrix& szCov)
      {
        CovarianceMatrix result = CovarianceMatrixUtil::stackBlocks(perigeeCov, szCov);
        return result;
      }
    };

    /// Vector of helix parameters
    using HelixParameters = HelixUtil::ParameterVector;

    /// Covariance matrix of the helix parameters
    using HelixCovariance = HelixUtil::CovarianceMatrix;

    /// Precision matrix of the helix parameters
    using HelixPrecision = HelixUtil::PrecisionMatrix;

    /// Jacobian matrix for the helix parameters
    using HelixJacobian = HelixUtil::JacobianMatrix;

  } // namespace TrackFindingCDC

} // namespace Belle2
