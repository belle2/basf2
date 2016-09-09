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

#include <tracking/trackFindingCDC/numerics/ParameterVector.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <Eigen/Dense>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// The representation type of derivatives from n to m parameters as used in the transport of covariance matrices.
    template <int M, int N = M>
    using JacobianMatrix = Eigen::Matrix<double, M, N>;

    /// Collection of functions related to jacobian matrices
    struct JacobianMatrixUtil {

      /// Construct a zero matrix
      template <unsigned int M, unsigned int N = M>
      static JacobianMatrix<M, N> zero()
      {
        return JacobianMatrix<M, N>::Zero();
      }

      /// Construct a unit matrix
      template <int M, int N = M>
      static JacobianMatrix<M, N> identity()
      {
        return JacobianMatrix<M, N>::Identity();
      }

      /// Construct a passive rotation matrix around coordinates i and j
      template <int N>
      static JacobianMatrix<N, N> passiveRotation(int i, int j, double phi)
      {
        return passiveRotation<N>(i, j, Vector2D::Phi(phi));
      }

      /// Construct a passive rotation matrix around coordinates i and j
      template <int N>
      static JacobianMatrix<N, N> passiveRotation(int i, int j, Vector2D phiVec)
      {
        assert(i < N);
        assert(j < N);
        JacobianMatrix<N, N> result = identity<N>();
        result(i, i) = phiVec.x();
        result(i, j) = phiVec.y();
        result(j, i) = -phiVec.y();
        result(j, j) = phiVec.x();
        return result;
      }

      /// Calculates the jacobian matrix for a scaling in each parameter.
      template <int N>
      static JacobianMatrix<N> scale(const ParameterVector<N>& scales)
      {
        return scales.asDiagonal();
      }

      /// Combines two jacobian matrices by putting them in two blocks diagonal to each other in a
      /// larger matrix
      template <int M1, int N1, int M2, int N2>
      static JacobianMatrix < M1 + M2, N1 + N2 > stackBlocks(const JacobianMatrix<M1, N1>& block1,
                                                             const JacobianMatrix<M2, N2>& block2)
      {
        JacobianMatrix < M1 + M2, N1 + N2 > result;
        result << block1, Eigen::Matrix<double, M1, N2>::Zero(),
               Eigen::Matrix<double, M2, N1>::Zero(), block2;
        return result;
      }
    };

  } // namespace TrackFindingCDC
} // namespace Belle2
