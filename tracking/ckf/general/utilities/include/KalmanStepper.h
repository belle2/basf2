/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/utilities/EigenHelper.h>
#include <framework/core/ModuleParamList.dcl.h>

#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/MeasurementOnPlane.h>

#include <Eigen/Dense>

namespace Eigen {
  /// A typical state vector
  using Vector5d = Matrix<double, 5, 1>;
  /// A typical cov matrix
  using Matrix5d = Matrix<double, 5, 5>;
}

namespace Belle2 {

  template <unsigned int Dimension>
  class KalmanStepper {
  public:
    double kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                      const genfit::MeasurementOnPlane& measurementOnPlane) const
    {
      // Extract matrices from the state
      Eigen::Vector5d x_k = convertToEigen<5>(measuredStateOnPlane.getState());
      Eigen::Matrix5d C_k = convertToEigen<5, 5>(measuredStateOnPlane.getCov());

      // extract matrices from the measurement
      const Eigen::Matrix<double, Dimension, 1>& m_k = convertToEigen<Dimension>(measurementOnPlane.getState());
      const Eigen::Matrix<double, Dimension, 5>& H_k = convertToEigen<Dimension, 5>(measurementOnPlane.getHMatrix()->getMatrix());
      const Eigen::Matrix<double, Dimension, Dimension>& V_k = convertToEigen<Dimension, Dimension>(measurementOnPlane.getCov());

      // This now is the real "update" step, where we update the X_k and the C_k.
      const Eigen::Matrix<double, 5, Dimension>& K_k = C_k * H_k.transpose() * (V_k + H_k * C_k * H_k.transpose()).inverse();
      C_k -= K_k * H_k * C_k;
      x_k += K_k * (m_k - H_k * x_k);

      // set the state back
      measuredStateOnPlane.setState(TVectorD(5, x_k.data()));
      measuredStateOnPlane.setCov(TMatrixDSym(5, C_k.data()));

      // We return the new chi2
      // TODO: the chi2 calculation includes another inversion, which may be optimized - however, this is not where the time is spent...
      const double chi2 = calculateChi2(x_k, C_k, m_k, H_k, V_k);
      return chi2;
    }

    double calculateChi2(const Eigen::Vector5d& x_k, const Eigen::Matrix5d& C_k,
                         const Eigen::Matrix<double, Dimension, 1>& m_k,
                         const Eigen::Matrix<double, Dimension, 5>& H_k,
                         const Eigen::Matrix<double, Dimension, Dimension>& V_k) const
    {
      const Eigen::Matrix<double, Dimension, 1>& residual = m_k - H_k * x_k;
      const double chi2 = (residual.transpose() * (V_k - H_k * C_k * H_k.transpose()).inverse() * residual).value();
      return chi2;
    }

    double calculateResidual(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                             const genfit::MeasurementOnPlane& measurementOnPlane) const
    {
      // Extract matrices from the state
      const Eigen::Vector5d& x_k = convertToEigen<5>(measuredStateOnPlane.getState());

      // extract matrices from the measurement
      const Eigen::Matrix<double, Dimension, 1>& m_k = convertToEigen<Dimension>(measurementOnPlane.getState());
      const Eigen::Matrix<double, Dimension, 5>& H_k = convertToEigen<Dimension, 5>(measurementOnPlane.getHMatrix()->getMatrix());

      const Eigen::Matrix<double, Dimension, 1>& residual = m_k - H_k * x_k;
      return residual.norm();
    }
  };
}