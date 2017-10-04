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

#include <tracking/trackFindingCDC/numerics/JacobianMatrix.h>

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

    // Guard to prevent repeated template symbol emission
    struct HelixUtil;
    extern template struct UncertainParametersUtil<HelixUtil, EHelixParameter>;

    /// Utility struct for functions and types related to the helix parameters.
    struct HelixUtil : UncertainParametersUtil<HelixUtil, EHelixParameter> {

      /// Getter for the signs which have to be applied to reverse the parameters
      static HelixUtil::ParameterVector reversalSigns();

      /// Get helix parameters related to the xy space
      static PerigeeUtil::ParameterVector getPerigeeParameters(const ParameterVector& helixPar);

      /// Get sz parameters
      static SZUtil::ParameterVector getSZParameters(const ParameterVector& helixPar);

      /// Combine parameters from the xy space and the sz space
      static HelixUtil::ParameterVector stack(const PerigeeUtil::ParameterVector& perigeePar,
                                              const SZUtil::ParameterVector& szPar);

      /// Get perigee covariance matrix related to the xy space
      static PerigeeUtil::CovarianceMatrix getPerigeeCovariance(const CovarianceMatrix& helixCov);

      /// Get sz covariance matrix
      static SZUtil::CovarianceMatrix getSZCovariance(const CovarianceMatrix& helixCov);

      /// Combine covariance matrices from the xy space and the sz space in their respective blocks
      static HelixUtil::CovarianceMatrix stackBlocks(const PerigeeUtil::CovarianceMatrix& perigeeCov,
                                                     const SZUtil::CovarianceMatrix& szCov);

      /// Matrix type for the ambiguity to the perigee parameters, e.g. under the stereo projection.
      using PerigeeAmbiguity = TrackFindingCDC::JacobianMatrix<3, 5>;

      /// Initialse a default covariance matrix to zero.
      static PerigeeAmbiguity defaultPerigeeAmbiguity();

      /**
       *  Calculates the weighted average between two helix parameter sets
       *  with their respective covariance matrix.
       *
       *  Returns the chi2 value of the average.
       */
      static double average(const HelixUtil::ParameterVector& fromPar,
                            const HelixUtil::CovarianceMatrix& fromCov,
                            const HelixUtil::ParameterVector& toPar,
                            const HelixUtil::CovarianceMatrix& toCov,
                            HelixUtil::ParameterVector& avgPar,
                            HelixUtil::CovarianceMatrix& avgCov);

      /**
       *  Calculates the weighted average between a perigee parameter sets and
       *  helix parameter sets with their respective covariance matrix under
       *  consideration of the relative ambiguity between the helix and
       *  the given perigee parameters.
       *
       *  Returns the chi2 value of the average.
       */
      static double average(const PerigeeUtil::ParameterVector& fromPar,
                            const PerigeeUtil::CovarianceMatrix& fromCov,
                            const HelixUtil::PerigeeAmbiguity& fromAmbiguity,
                            const HelixUtil::ParameterVector& toPar,
                            const HelixUtil::CovarianceMatrix& toCov,
                            HelixUtil::ParameterVector& avgPar,
                            HelixUtil::CovarianceMatrix& avgCov);

      /**
       *  Calculates the weighted average between two perigee parameter sets
       *  with their respective covariance matrix under consideration of their relative
       *  ambiguity to a helix near the sz parameters given as a reference.
       *
       *  Returns the chi2 value of the average.
       */
      static double average(const PerigeeUtil::ParameterVector& fromPar,
                            const PerigeeUtil::CovarianceMatrix& fromCov,
                            const HelixUtil::PerigeeAmbiguity& fromAmbiguity,
                            const PerigeeUtil::ParameterVector& toPar,
                            const PerigeeUtil::CovarianceMatrix& toCov,
                            const HelixUtil::PerigeeAmbiguity& toAmbiguity,
                            const SZUtil::ParameterVector& szParameters,
                            HelixUtil::ParameterVector& avgPar,
                            HelixUtil::CovarianceMatrix& avgCov);
    };

    /// Vector of helix parameters
    using HelixParameters = HelixUtil::ParameterVector;

    /// Covariance matrix of the helix parameters
    using HelixCovariance = HelixUtil::CovarianceMatrix;

    /// Precision matrix of the helix parameters
    using HelixPrecision = HelixUtil::PrecisionMatrix;

    /// Jacobian matrix for the helix parameters
    using HelixJacobian = HelixUtil::JacobianMatrix;

    /// Matrix type for the ambiguity to the perigee parameters, e.g. under the stereo projection.
    using PerigeeHelixAmbiguity = HelixUtil::PerigeeAmbiguity;
  }

}
