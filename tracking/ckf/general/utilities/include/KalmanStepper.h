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

#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/MeasurementOnPlane.h>

#include <Eigen/Dense>


namespace Belle2 {

  /**
   * Class to bundle all algorithms needed for the kalman update procedure.
   *
   * Its main functionality is to update a measured state on plane with a measurement on plane from a hit
   * using the Kalman procedure described in https://doi.org/10.1016/0168-9002(87)90887-4.
   *
   * @tparam Dimension The dimension of the hit - e.g. how many Kalman state parameters are defined by the hits.
   *   This defines the size of the matrices.
   */
  template <unsigned int Dimension>
  class KalmanStepper {
    /// Matrix class Dimension x 1
    using MeasurementState = Eigen::Matrix<double, Dimension, 1>;
    /// Matrix class Dimension x Dimension
    using MeasurementCovariance = Eigen::Matrix<double, Dimension, Dimension>;
    /// Matrix class Dimension x 5
    using HMatrix = Eigen::Matrix<double, Dimension, 5>;
    /// Matrix class 5 x 1
    using TrackState = Eigen::Matrix<double, 5, 1>;
    /// Matrix class 5 x 5
    using TrackCovariance = Eigen::Matrix<double, 5, 5>;

  public:
    /// Kalman update of the mSoP using the measurement. Is just a wrapper around the other kalmanStepper working with bare matrices.
    double kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                      const genfit::MeasurementOnPlane& measurementOnPlane) const
    {
      // Extract matrices from the state
      TrackState x_k = convertToEigen<5>(measuredStateOnPlane.getState());
      TrackCovariance C_k = convertToEigen<5, 5>(measuredStateOnPlane.getCov());

      // extract matrices from the measurement
      const MeasurementState& m_k = convertToEigen<Dimension>(measurementOnPlane.getState());
      const HMatrix& H_k = convertToEigen<Dimension, 5>(measurementOnPlane.getHMatrix()->getMatrix());
      const MeasurementCovariance& V_k = convertToEigen<Dimension, Dimension>(measurementOnPlane.getCov());

      const double chi2 = kalmanStep(x_k, C_k, m_k, V_k, H_k);

      // set the state back
      measuredStateOnPlane.setState(TVectorD(5, x_k.data()));
      measuredStateOnPlane.setCov(TMatrixDSym(5, C_k.data()));

      return chi2;
    }

    /// Helper function to calculate a residual between the mSoP and the measurement.
    double calculateResidual(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                             const genfit::MeasurementOnPlane& measurementOnPlane) const
    {
      // Extract matrices from the state
      const TrackState& x_k = convertToEigen<5>(measuredStateOnPlane.getState());

      // extract matrices from the measurement
      const MeasurementState& m_k = convertToEigen<Dimension>(measurementOnPlane.getState());
      const HMatrix& H_k = convertToEigen<Dimension, 5>(measurementOnPlane.getHMatrix()->getMatrix());

      const MeasurementState& residual = m_k - H_k * x_k;
      return residual.norm();
    }

  private:
    /// This now is the real "update" step, where we update the X_k and the C_k.
    double kalmanStep(TrackState& x_k, TrackCovariance& C_k,
                      const MeasurementState& m_k,
                      const MeasurementCovariance& V_k,
                      const HMatrix& H_k) const
    {
      const Eigen::Matrix<double, 5, Dimension>& K_k = C_k * H_k.transpose() * (V_k + H_k * C_k * H_k.transpose()).inverse();
      C_k -= K_k * H_k * C_k;
      x_k += K_k * (m_k - H_k * x_k);

      const MeasurementState& residual = m_k - H_k * x_k;
      const double chi2 = (residual.transpose() * (V_k - H_k * C_k * H_k.transpose()).inverse() * residual).value();
      return chi2;
    }
  };
}