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

#include <tracking/trackFindingCDC/geometry/CovarianceMatrix.h>
#include <tracking/trackFindingCDC/geometry/ParameterVector.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Utility class template the instantiate many helper function related to a uncertain parameter
    /// vector
    template <class T, class AEParameters>
    struct UncertainParametersUtil {

      /// Indices of the parameters
      using EParamaters = AEParameters;

      /// Number of Elements
      static const size_t c_N = EParamaters::c_N;

      /// The vector type representing the n related parameters.
      using ParameterVector = TrackFindingCDC::ParameterVector<c_N>;

      /// The matrix type implementing the representation of the covariance
      using CovarianceMatrix = TrackFindingCDC::CovarianceMatrix<c_N>;

      /// The matrix type implementing the representation of the precision
      using PrecisionMatrix = TrackFindingCDC::PrecisionMatrix<c_N>;

      /// The matrix type implementing the representation of a jacobian matrix used in the transport
      /// operations
      using JacobianMatrix = TrackFindingCDC::JacobianMatrix<c_N>;

      /// Transport the covariance matrix inplace with the given jacobian matrix
      static void transport(const JacobianMatrix& jacobian, CovarianceMatrix& cov)
      {
        CovarianceMatrixUtil::transport(jacobian, cov);
      }

      /// Return a copy of the covariance matrix transported with the given jacobian matrix
      static CovarianceMatrix transported(const JacobianMatrix& jacobian,
                                          const CovarianceMatrix& cov)
      {
        return CovarianceMatrixUtil::transported(jacobian, cov);
      }

      /// Jacobian matrix needed in the reversal operation of the parameter vector
      static JacobianMatrix reversalJacobian()
      {
        return JacobianMatrixUtil::scale(T::reversalSigns());
      }

      /// Reverse the covariance matrix inplace.
      static void reverse(CovarianceMatrix& cov)
      {
        return transport(reversalJacobian(), cov);
      }

      /// Return a copy of the reversed covariance matrix.
      static CovarianceMatrix reversed(const CovarianceMatrix& cov)
      {
        return transported(reversalJacobian(), cov);
      }

      /// Returns an identity matrix
      static CovarianceMatrix identity()
      {
        return CovarianceMatrixUtil::identity<c_N>();
      }

      /// Getter for a sub part of the covariance matrix.
      template <class AParameterVector, unsigned int I = 0>
      static AParameterVector getSubParameterVector(const ParameterVector& par)
      {
        return ParameterVectorUtil::getSub<AParameterVector, I>(par);
      }

      /// Getter for a sub part of the covariance matrix.
      template <class ACovarianceMatrix, unsigned int I = 0>
      static ACovarianceMatrix getSubCovarianceMatrix(const CovarianceMatrix& cov)
      {
        return CovarianceMatrixUtil::getSub<ACovarianceMatrix, I>(cov);
      }
    };

  } // namespace TrackFindingCDC
} // namespace Belle2
