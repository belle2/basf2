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

#include <tracking/trackFindingCDC/numerics/JacobianMatrixUtil.h>

#include <tracking/trackFindingCDC/numerics/PrecisionMatrix.h>
#include <tracking/trackFindingCDC/numerics/JacobianMatrix.h>
#include <tracking/trackFindingCDC/numerics/ParameterVector.h>

#include <tracking/trackFindingCDC/numerics/EigenView.h>

#include <Eigen/Core>

#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Collection of functions related to precision matrices
    struct PrecisionMatrixUtil {

      /// Constructs an identity matrix
      template <int N>
      static PrecisionMatrix<N> identity()
      {
        return PrecisionMatrix<N>::Identity();
      }

      /// Transport the precision matrix inplace with the given jacobian matrix
      template <int N>
      static void transport(const JacobianMatrix<N, N>& ambiguity, PrecisionMatrix<N>& precision)
      {
        mapToEigen(precision) =
          mapToEigen(ambiguity).transpose() * mapToEigen(precision) * mapToEigen(ambiguity);
      }

      /// Return a copy of the precision matrix transported with the given back projection jacobian
      /// matrix
      template <int M, int N>
      static PrecisionMatrix<M> transported(const JacobianMatrix<N, M>& ambiguity,
                                            const PrecisionMatrix<N>& precision)
      {
        return mapToEigen(ambiguity).transpose() * mapToEigen(precision) * mapToEigen(ambiguity);
      }

      /// Scale the precision inplace by the given factors in each parameter
      template <int N>
      static void scale(const ParameterVector<N>& scales, PrecisionMatrix<N>& precision)
      {
        return transport(JacobianMatrixUtil::scale(1 / scales), precision);
      }

      /// Return a copy of the precision scaled by the given factors in each parameter
      template <int N>
      static PrecisionMatrix<N> scale(const ParameterVector<N>& scales,
                                      const PrecisionMatrix<N>& precision)
      {
        return transported(JacobianMatrixUtil::scale(1 / scales), precision);
      }

      /// Combines two precision matrices by putting them in two blocks on the diagonal of a larger
      /// matrix
      template <int N1, int N2>
      static PrecisionMatrix < N1 + N2 > stackBlocks(const PrecisionMatrix<N1>& block1,
                                                     const PrecisionMatrix<N2>& block2)
      {
        PrecisionMatrix < N1 + N2 > result;
        mapToEigen(result) << mapToEigen(block1), Eigen::Matrix<double, N1, N2>::Zero(),
                   Eigen::Matrix<double, N2, N1>::Zero(), mapToEigen(block2);
        return result;
      }

      /// Gets a subprecision from a precision matrix
      template <class APrecisionMatrix, int I = 0, int N = 0>
      static APrecisionMatrix getSub(const PrecisionMatrix<N>& precision)
      {
        constexpr const int M =
          std::remove_reference_t<decltype(mapToEigen(APrecisionMatrix()))>::RowsAtCompileTime;
        return precision.template block<M, M>(I, I);
      }

      /**
       *  Averages two parameter vectors taking into account their respective precision.
       *
       *  @param      parameter1  First parameters
       *  @param      precision1  Precision matrix to the first parameters
       *  @param      parameter2  Second parameters
       *  @param      precision2  Precision matrix to the second parameters
       *  @param[out] parameter   Averaged parameters
       *  @param[out] precision   Averaged precision matrix
       *  @return                 Chi square deviation of orignal parameters to the average
       */
      template <int N>
      static double average(const ParameterVector<N>& parameter1,
                            const PrecisionMatrix<N>& precision1,
                            const ParameterVector<N>& parameter2,
                            const PrecisionMatrix<N>& precision2,
                            ParameterVector<N>& parameter,
                            PrecisionMatrix<N>& precision)
      {
        const auto& ePrecision1 = mapToEigen(precision1);
        const auto& ePrecision2 = mapToEigen(precision2);
        auto&& ePrecision = mapToEigen(precision);

        const auto& eParameter1 = mapToEigen(parameter1);
        const auto& eParameter2 = mapToEigen(parameter2);
        auto&& eParameter = mapToEigen(parameter);

        ePrecision = ePrecision1 + ePrecision2;
        eParameter = ePrecision.colPivHouseholderQr().solve(ePrecision1 * eParameter1 +
                                                            ePrecision2 * eParameter2);

        auto eResidual1 = eParameter1 - eParameter;
        auto eResidual2 = eParameter2 - eParameter;

        Eigen::Matrix<double, 1, 1> chi2 = (eResidual1.transpose() * ePrecision1 * eResidual1 +
                                            eResidual2.transpose() * ePrecision2 * eResidual2);
        return chi2[0];
      }

      /**
       *  Averages two parameter vectors from a projected space taking into account their respective
       * precisions and ambiguity matrices
       *
       *  @param      parameter1  First parameters
       *  @param      precision1  Precision matrix to the first parameters
       *  @param      ambiguity1  Projection ambiguity of the first parameters
       *  @param      parameter2  Second parameters
       *  @param      precision2  Precision matrix to the second parameters
       *  @param      ambiguity2  Projection ambiguity of the second parameters
       *  @param[out] parameter   Averaged parameters
       *  @param[out] precision   Averaged precision matrix
       *  @return                 Chi square deviation of original parameters to the average
       */
      template <int M, int N1, int N2>
      static double average(const ParameterVector<N1>& parameter1,
                            const PrecisionMatrix<N1>& precision1,
                            const JacobianMatrix<N1, M>& ambiguity1,
                            const ParameterVector<N2>& parameter2,
                            const PrecisionMatrix<N2>& precision2,
                            const JacobianMatrix<N2, M>& ambiguity2,
                            ParameterVector<M>& parameter,
                            PrecisionMatrix<M>& precision)
      {
        const auto& eParameter1 = mapToEigen(parameter1);
        const auto& ePrecision1 = mapToEigen(precision1);
        const auto& eAmbiguity1 = mapToEigen(ambiguity1);

        const auto& eParameter2 = mapToEigen(parameter2);
        const auto& ePrecision2 = mapToEigen(precision2);
        const auto& eAmbiguity2 = mapToEigen(ambiguity2);

        auto&& ePrecision = mapToEigen(precision);
        auto&& eParameter = mapToEigen(parameter);

        ePrecision = (eAmbiguity1.transpose() * ePrecision1 * eAmbiguity1 +
                      eAmbiguity2.transpose() * ePrecision2 * eAmbiguity2);

        eParameter = ePrecision.colPivHouseholderQr().solve(
                       eAmbiguity1.transpose() * ePrecision1 * eParameter1 +
                       eAmbiguity2.transpose() * ePrecision2 * eParameter2);

        auto eResidual1 = eParameter1 - eAmbiguity1 * eParameter;
        auto eResidual2 = eParameter2 - eAmbiguity2 * eParameter;

        Eigen::Matrix<double, 1, 1> chi2 = (eResidual1.transpose() * ePrecision1 * eResidual1 +
                                            eResidual2.transpose() * ePrecision2 * eResidual2);
        return chi2[0];
      }
    };
  }
}
