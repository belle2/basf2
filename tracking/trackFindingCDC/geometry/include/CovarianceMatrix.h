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

#include <tracking/trackFindingCDC/geometry/ParameterVector.h>
#include <tracking/trackFindingCDC/geometry/JacobianMatrix.h>


#include <Eigen/Dense>
#include <TMatrixDSym.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// The representation type of a covariance matrix
    template<int N>
    using CovarianceMatrix = Eigen::Matrix<double, N, N>;

    /// The representation type of an inverse covariance matrix
    template<int N>
    using WeightMatrix = Eigen::Matrix<double, N, N>;

    /// Collection of functions related to covariance matrices
    struct CovarianceMatrixUtil {

      /// Constructs an identity matrix
      template<int N>
      static CovarianceMatrix<N> identity()
      { return CovarianceMatrix<N>::Identity(); }

      /// Create a covariance matrix from a TMatrix representation
      template<int N>
      static CovarianceMatrix<N> fromTMatrix(const TMatrixDSym& tCov)
      {
        assert(tCov.GetNrows() == N);
        assert(tCov.GetNcols() == N);
        assert(tCov.GetRowLwb() == 0);
        assert(tCov.GetColLwb() == 0);

        CovarianceMatrix<N> result;
        for (int i = 0; i < N; ++i) {
          for (int j = 0; j < N; ++j) {
            result(i, j) = tCov(i, j);
          }
        }
        return result;
      }

      /// Translate the covariance matrix to the TMatrix representation
      template<int N>
      static TMatrixDSym toTMatrix(const CovarianceMatrix<N>& cov)
      {
        TMatrixDSym result(N);
        for (int i = 0; i < N; ++i) {
          for (int j = 0; j < N; ++j) {
            result(i, j) = cov(i, j);
          }
        }
        return result;
      }


      /// Transport the covariance matrix inplace with the given jacobian matrix
      template<int N>
      static void transport(const JacobianMatrix<N, N>& jacobian,
                            CovarianceMatrix<N>& cov)
      { cov = jacobian * cov * jacobian.transpose(); }

      /// Return a copy of the covariance matrix transported with the given jacobian matrix
      template<int M, int N>
      static CovarianceMatrix<M> transported(const JacobianMatrix<M, N>& jacobian,
                                             const CovarianceMatrix<N>& cov)
      { return jacobian * cov * jacobian.transpose(); }

      /// Scale the covariance inplace by the given factors in each parameter
      template<int N>
      static void scale(const ParameterVector<N>& scales,
                        CovarianceMatrix<N>& cov)
      { return transport(JacobianMatrixUtil::scale(scales), cov); }

      /// Return a copy of the covariance scaled by the given factors in each parameter
      template<int N>
      static CovarianceMatrix<N> scale(const ParameterVector<N>& scales,
                                       const CovarianceMatrix<N>& cov)
      { return transported(JacobianMatrixUtil::scale(scales), cov); }

      /// Combines two covariance matrices by putting them in two blocks on the diagonal of a larger matrix
      template<int N1, int N2>
      static CovarianceMatrix < N1 + N2 > stackBlocks(const CovarianceMatrix<N1>& block1,
                                                      const CovarianceMatrix<N2>& block2)
      {
        CovarianceMatrix < N1 + N2 > result;
        result <<
               block1, Eigen::Matrix<double, N1, N2>::Zero(),
                       Eigen::Matrix<double, N2, N1>::Zero(), block2;
        return result;
      }

      /// Gets a subcovariance from a covariance matrix
      template<class ACovarianceMatrix, int I = 0, int N = 0>
      static ACovarianceMatrix getSub(const CovarianceMatrix<N>& cov)
      {
        constexpr const int M  = ACovarianceMatrix::RowsAtCompileTime;
        return cov.template block<M, M>(I, I);
      }

      /**
       *  Averages two parameter vectors taking into account their respective covariances.
       *
       *  @param par1     First parameters
       *  @param cov1     Covariance matrix to the first parameters
       *  @param par2     Second parameters
       *  @param cov2     Covariance matrix to the second parameters
       *  @param par[out] Averaged parameters
       *  @param cov[out] Averaged covariance matrix
       *  @return         Chi square deviation of orignal parameters to the average
       */
      template<int N>
      static double average(const ParameterVector<N>& par1,
                            const CovarianceMatrix<N>& cov1,
                            const ParameterVector<N>& par2,
                            const CovarianceMatrix<N>& cov2,
                            ParameterVector<N>& par,
                            CovarianceMatrix<N>& cov)
      {
        WeightMatrix<N> weight1 = cov1.inverse();
        WeightMatrix<N> weight2 = cov2.inverse();
        WeightMatrix<N> weight =  weight1 + weight2;
        cov = weight.inverse();

        par = cov * (weight1 * par1 + weight2 * par2);

        ParameterVector<N> residual1 = par1 - par;
        ParameterVector<N> residual2 = par2 - par;

        Eigen::Matrix<double, 1, 1> chi2 = residual1.transpose() * weight1 * residual1 + residual2.transpose() * weight2 * residual2;
        return chi2[0];
      }

      /**
       *  Averages two parameter vectors from a projected space taking into account their respective covariances and ambiguity matrices
       *
       *  @param par1       First parameters
       *  @param cov1       Covariance matrix to the first parameters
       *  @param ambiguity1 Projection ambiguity of the first parameters
       *  @param par2       Second parameters
       *  @param cov2       Covariance matrix to the second parameters
       *  @param ambiguity2 Projection ambiguity of the second parameters
       *  @param par[out]   Averaged parameters
       *  @param cov[out]   Averaged covariance matrix
       *  @return           Chi square deviation of original parameters to the average
       */
      template<int M, int N1, int N2>
      static double average(const ParameterVector<N1>& par1,
                            const CovarianceMatrix<N1>& cov1,
                            const JacobianMatrix<N1, M>& ambiguity1,
                            const ParameterVector<N2>& par2,
                            const CovarianceMatrix<N2>& cov2,
                            const JacobianMatrix<N2, M>& ambiguity2,
                            ParameterVector<M>& par,
                            CovarianceMatrix<M>& cov)
      {
        WeightMatrix<N1> weight1 = cov1.inverse();
        WeightMatrix<N2> weight2 = cov2.inverse();

        WeightMatrix<M> weight = ambiguity1.transpose() * weight1 * ambiguity1 + ambiguity2.transpose() * weight2 * ambiguity2;
        cov = weight.inverse();

        par = cov * (ambiguity1.transpose() * weight1 * par1 + ambiguity2.transpose() * weight2 * par2);
        ParameterVector<N1> residual1 = par1 - ambiguity1 * par;
        ParameterVector<N2> residual2 = par2 - ambiguity2 * par;

        Eigen::Matrix<double, 1, 1> chi2 = residual1.transpose() * weight1 * residual1 + residual2.transpose() * weight2 * residual2;
        return chi2[0];
      }
    };

  } // namespace TrackFindingCDC
} // namespace Belle2
