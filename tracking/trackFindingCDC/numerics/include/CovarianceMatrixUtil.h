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

#include <tracking/trackFindingCDC/numerics/EigenView.h>

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/QR>

#include <type_traits>
#include <cmath>
#include <cassert>

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
        mapToEigen(cov) = mapToEigen(jacobian) * mapToEigen(cov) * mapToEigen(jacobian).transpose();
      }

      /// Return a copy of the covariance matrix transported with the given jacobian matrix
      template <int M, int N>
      static CovarianceMatrix<M> transported(const JacobianMatrix<M, N>& jacobian,
                                             const CovarianceMatrix<N>& cov)
      {
        CovarianceMatrix<M> result;
        mapToEigen(result) = mapToEigen(jacobian) * mapToEigen(cov) * mapToEigen(jacobian).transpose();
        return result;
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
        mapToEigen(result) << mapToEigen(block1), Eigen::Matrix<double, N1, N2>::Zero(),
                   Eigen::Matrix<double, N2, N1>::Zero(), mapToEigen(block2);
        return result;
      }

      /// Gets a subcovariance from a covariance matrix
      template <class ACovarianceMatrix, int I = 0, int N = 0>
      static ACovarianceMatrix getSub(const CovarianceMatrix<N>& cov)
      {
        constexpr const int M =
          std::remove_reference_t<decltype(mapToEigen(ACovarianceMatrix()))>::RowsAtCompileTime;
        return cov.template block<M, M>(I, I);
      }

      /// Convert precision matrix to covariance matrix - allows for reduced rank
      template <int N>
      static CovarianceMatrix<N> fromPrecision(const PrecisionMatrix<N>& prec)
      {
        CovarianceMatrix<N> cov;
        mapToEigen(cov) = mapToEigen(prec).colPivHouseholderQr().inverse();
        return cov;
      }

      /// Convert covariance matrix to precision matrix - allows for reduced rank
      template <int N>
      static PrecisionMatrix<N> toPrecision(const CovarianceMatrix<N>& cov)
      {
        PrecisionMatrix<N> prec;
        mapToEigen(prec) = mapToEigen(cov).colPivHouseholderQr().inverse();
        return prec;
      }

      /// Convert covariance matrix to precision matrix - assumes full rank
      template <int N>
      static CovarianceMatrix<N> fromFullPrecision(const PrecisionMatrix<N>& prec)
      {
        CovarianceMatrix<N> cov;
        mapToEigen(cov) = mapToEigen(prec).inverse();
        return cov;
      }

      /// Convert precision matrix to covariance matrix - assumes full rank
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
       *  Averages two parameter vectors, one being from a projected space,
       *  taking into account their respective covariances and the ambiguity
       *  of the projection to the actual state
       *
       *  @param      par1        First parameters
       *  @param      cov1        Covariance matrix to the first parameters
       *  @param      ambiguity1  Projection ambiguity of the first parameters
       *  @param      par2        Second parameters
       *  @param      cov2        Covariance matrix to the second parameters
       *  @param[out] par         Averaged parameters
       *  @param[out] cov         Averaged covariance matrix
       *  @return                 Chi square deviation of orignal parameters to the average
       */
      template <int M, int N1>
      static double average(const ParameterVector<N1>& par1,
                            const CovarianceMatrix<N1>& cov1,
                            const JacobianMatrix<N1, M>& ambiguity1,
                            const ParameterVector<M>& par2,
                            const CovarianceMatrix<M>& cov2,
                            ParameterVector<M>& par,
                            CovarianceMatrix<M>& cov)
      {
        /* Naive implementation */
        JacobianMatrix<M, M> ambiguity2 = identity<M>();
        return average(par1, cov1, ambiguity1, par2, cov2, ambiguity2, par, cov);

        /* Kalman implementation - may be faster */
        // par = par2;
        // cov = cov2;
        // return kalmanUpdate(par1, cov1, ambiguity1, par, cov);
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

      /**
       *  Updates a parameter and its covariance by integrating information from parameter vector
       *  in a projected space and its covariances.
       *  The update is expressed in terms of Kalman's equations.
       *
       *  This function updates the values of the second parameters in place.
       *
       *  @param          par1        First parameters
       *  @param          cov1        Covariance matrix to the first parameters
       *  @param          ambiguity1  Projection ambiguity of the first parameters
       *  @param[in, out] par2        Second parameters - updated inplace
       *  @param[in, out] cov2        Second covariance matrix - updated inplace
       *  @return                     Chi square deviation of orignal parameters to the average
       */
      template <int M, int N1>
      static double kalmanUpdate(const ParameterVector<N1>& par1,
                                 const CovarianceMatrix<N1>& cov1,
                                 const JacobianMatrix<N1, M>& ambiguity1,
                                 ParameterVector<M>& par2,
                                 CovarianceMatrix<M>& cov2)
      {
        /* Kalman update - smart version */
        const auto& ePar1 = mapToEigen(par1);
        const auto& eCov1 = mapToEigen(cov1);
        const auto& eAmbiguity1 = mapToEigen(ambiguity1);

        auto&& ePar2 = mapToEigen(par2);
        auto&& eCov2 = mapToEigen(cov2);

        // Apriori residual
        auto residual = ePar1 - eAmbiguity1 * ePar2;
        auto residualCov = eCov1 + eAmbiguity1 * eCov2 * eAmbiguity1.transpose();
        auto residualPrecision = residualCov.inverse();

        // Chi2 - calculate on the apriori residuals, which is the same value as for the posterior.
        Eigen::Matrix<double, 1, 1> chi2 = residual.transpose() * residualPrecision * residual;

        // Update parameters
        auto kalmanGain = eCov2 * eAmbiguity1.transpose() * residualPrecision;
        eCov2 -= kalmanGain * eAmbiguity1 * eCov2;
        ePar2 += kalmanGain * residual;

        return chi2[0];

        // Posterior residual - here for reference, should be compiled out.
        auto postResidual = ePar1 - eAmbiguity1 * ePar2;
        auto postResidualCov = eCov1 - eAmbiguity1 * eCov2 * eAmbiguity1.transpose();
        auto postResidualPrecision = postResidualCov.inverse();
        Eigen::Matrix<double, 1, 1> postChi2 =
          postResidual.transpose() * postResidualPrecision * postResidual;
        (void)postChi2;
        assert(std::fabs(postChi2[0] - chi2[0]) < 1e-8);
      }
    };

  }
}
