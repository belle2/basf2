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

#include <tracking/trackFindingCDC/numerics/PrecisionMatrixUtil.h>
#include <tracking/trackFindingCDC/numerics/JacobianMatrixUtil.h>

#include <tracking/trackFindingCDC/numerics/CovarianceMatrix.h>
#include <tracking/trackFindingCDC/numerics/PrecisionMatrix.h>
#include <tracking/trackFindingCDC/numerics/JacobianMatrix.h>
#include <tracking/trackFindingCDC/numerics/ParameterVector.h>

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/QR>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Collection of functions related to covariance matrices
    struct CovarianceMatrixUtil {

      /// Constructs an identity matrix
      template <int N>
      static CovarianceMatrix<N> identity()
      {
        return CovarianceMatrix<N>::Identity();
      }

      /// Transport the covariance matrix inplace with the given jacobian matrix
      template <int N>
      static void transport(const JacobianMatrix<N, N>& jacobian, CovarianceMatrix<N>& cov)
      {
        cov = jacobian * cov * jacobian.transpose();
      }

      /// Return a copy of the covariance matrix transported with the given jacobian matrix
      template <int M, int N>
      static CovarianceMatrix<M> transported(const JacobianMatrix<M, N>& jacobian,
                                             const CovarianceMatrix<N>& cov)
      {
        return jacobian * cov * jacobian.transpose();
      }

      /// Scale the covariance inplace by the given factors in each parameter
      template <int N>
      static void scale(const ParameterVector<N>& scales, CovarianceMatrix<N>& cov)
      {
        return transport(JacobianMatrixUtil::scale(scales), cov);
      }

      /// Return a copy of the covariance scaled by the given factors in each parameter
      template <int N>
      static CovarianceMatrix<N> scale(const ParameterVector<N>& scales,
                                       const CovarianceMatrix<N>& cov)
      {
        return transported(JacobianMatrixUtil::scale(scales), cov);
      }

      /// Combines two covariance matrices by putting them in two blocks on the diagonal of a larger
      /// matrix
      template <int N1, int N2>
      static CovarianceMatrix < N1 + N2 > stackBlocks(const CovarianceMatrix<N1>& block1,
                                                      const CovarianceMatrix<N2>& block2)
      {
        CovarianceMatrix < N1 + N2 > result;
        result << block1, Eigen::Matrix<double, N1, N2>::Zero(),
               Eigen::Matrix<double, N2, N1>::Zero(), block2;
        return result;
      }

      /// Gets a subcovariance from a covariance matrix
      template <class ACovarianceMatrix, int I = 0, int N = 0>
      static ACovarianceMatrix getSub(const CovarianceMatrix<N>& cov)
      {
        constexpr const int M = ACovarianceMatrix::RowsAtCompileTime;
        return cov.template block<M, M>(I, I);
      }

      template <int N>
      static CovarianceMatrix<N> fromPrecision(const PrecisionMatrix<N>& prec)
      {
        return prec.colPivHouseholderQr().inverse();
      }

      template <int N>
      static PrecisionMatrix<N> toPrecision(const CovarianceMatrix<N>& cov)
      {
        return cov.colPivHouseholderQr().inverse();
      }

      template <int N>
      static CovarianceMatrix<N> fromFullPrecision(const PrecisionMatrix<N>& prec)
      {
        CovarianceMatrix<N> cov;
        mapToEigen(cov) = mapToEigen(prec).inverse();
        return cov;
      }

      template <int N>
      static PrecisionMatrix<N> fullToPrecision(const CovarianceMatrix<N>& cov)
      {
        PrecisionMatrix<N> prec;
        mapToEigen(prec) = mapToEigen(cov).inverse();
        return prec;
      }

      /**
       *  Averages two parameter vectors taking into account their respective covariances.
       *
       *  @param      par1  First parameters
       *  @param      cov1  Covariance matrix to the first parameters
       *  @param      par2  Second parameters
       *  @param      cov2  Covariance matrix to the second parameters
       *  @param[out] par   Averaged parameters
       *  @param[out] cov   Averaged covariance matrix
       *  @return           Chi square deviation of orignal parameters to the average
       */
      template <int N>
      static double average(const ParameterVector<N>& par1,
                            const CovarianceMatrix<N>& cov1,
                            const ParameterVector<N>& par2,
                            const CovarianceMatrix<N>& cov2,
                            ParameterVector<N>& par,
                            CovarianceMatrix<N>& cov)
      {
        PrecisionMatrix<N> precision1 = toPrecision(cov1);
        PrecisionMatrix<N> precision2 = toPrecision(cov2);
        PrecisionMatrix<N> precision;
        double chi2 =
          PrecisionMatrixUtil::average(par1, precision1, par2, precision2, par, precision);
        cov = fromPrecision(precision);
        return chi2;
      }

      /**
       *  Averages two parameter vectors from a projected space taking into account their respective
       *  covariances and ambiguity matrices.
       *
       *  @param      par1        First parameters
       *  @param      cov1        Covariance matrix to the first parameters
       *  @param      ambiguity1  Projection ambiguity of the first parameters
       *  @param      par2        Second parameters
       *  @param      cov2        Covariance matrix to the second parameters
       *  @param      ambiguity2  Projection ambiguity of the second parameters
       *  @param[out] par         Averaged parameters
       *  @param[out] cov         Averaged covariance matrix
       *  @return                 Chi square deviation of original parameters to the average
       */
      template <int M, int N1, int N2>
      static double average(const ParameterVector<N1>& par1,
                            const CovarianceMatrix<N1>& cov1,
                            const JacobianMatrix<N1, M>& ambiguity1,
                            const ParameterVector<N2>& par2,
                            const CovarianceMatrix<N2>& cov2,
                            const JacobianMatrix<N2, M>& ambiguity2,
                            ParameterVector<M>& par,
                            CovarianceMatrix<M>& cov)
      {
        PrecisionMatrix<N1> precision1 = toPrecision(cov1);
        PrecisionMatrix<N2> precision2 = toPrecision(cov2);
        PrecisionMatrix<M> precision;
        double chi2 = PrecisionMatrixUtil::average(par1,
                                                   precision1,
                                                   ambiguity1,
                                                   par2,
                                                   precision2,
                                                   ambiguity2,
                                                   par,
                                                   precision);
        cov = fromPrecision(precision);
        return chi2;
      }
    };

  }
}
