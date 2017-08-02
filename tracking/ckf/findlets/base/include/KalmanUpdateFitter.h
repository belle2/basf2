/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/CompositeProcessingSignalListener.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/MeasurementOnPlane.h>

#include <Eigen/Dense>

namespace Eigen {
  using Vector5d = Matrix<double, 5, 1>;
  using RowVector5d = Matrix<double, 1, 5>;
  using Vector1d = Matrix<double, 1, 1>;
  using Matrix5d = Matrix<double, 5, 5>;
}

namespace Belle2 {
  class SpacePoint;

  namespace TrackFindingCDC {
    class CDCRLWireHit;
  }

  /**
   * Algorithm class for updating the track parameters and the covariance matrix based on the Kalman algorithm.
   *
   * For this, the state has to be already extrapolated to the plane of the space point.
   */
  class KalmanUpdateFitter : public TrackFindingCDC::CompositeProcessingSignalListener {
  public:
    /// Reusable function to do the kalman update of a mSoP with the information in the hit. Some implementations further down.
    template <class ARecoHit>
    double kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const;

    /// Currently, no parameters are exported.
    void exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                          const std::string& prefix __attribute__((unused)))
    {}

    /// Main function: update the parameters stored in the mSoP of the state using the SP related to this state.
    template <class AStateObject>
    TrackFindingCDC::Weight operator()(AStateObject& currentState) const;
  };

  template <class AStateObject>
  TrackFindingCDC::Weight KalmanUpdateFitter::operator()(AStateObject& currentState) const
  {
    B2ASSERT("Encountered invalid state", not currentState.isFitted() and currentState.isAdvanced());

    const auto* hit = currentState.getHit();

    if (not hit) {
      // If we do not have a space point, we do not need to do anything here.
      currentState.setFitted();
      return 1;
    }

    // This is the extrapolated measured state on plane, which is defined on the same plane as the hit
    // If it would not be defined, the extrapolation would have gone wrong, so we would not end up here.
    // Or we do not have a hit at all, which is already checked above.
    genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState.getMeasuredStateOnPlane();

    // Do the kalman step and return the chi2 of this step
    const double chi2 = kalmanStep(measuredStateOnPlane, *hit);

    currentState.setMeasuredStateOnPlane(measuredStateOnPlane);
    currentState.setChi2(chi2);
    currentState.setFitted();
    return 1;
  }

  template <>
  double KalmanUpdateFitter::kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const SpacePoint& recoHit) const;

  template <>
  double KalmanUpdateFitter::kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                        const TrackFindingCDC::CDCRLWireHit& rlWireHit) const;

  template <class ARecoHit>
  double KalmanUpdateFitter::kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const
  {
    // We will change the state x_k, the covariance C_k and the chi2
    Eigen::Vector5d x_k_old(measuredStateOnPlane.getState().GetMatrixArray());
    Eigen::Matrix5d C_k_old(measuredStateOnPlane.getCov().GetMatrixArray());

    // Important: measuredStateOnPlane must already be extrapolated to the correct plane.
    // Only the plane and the rep are accessed (but the rep has no meaningful members).
    const std::vector<genfit::MeasurementOnPlane*> measurementsOnPlane = recoHit.constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("There should be exactly one measurement on plane", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane& measurementOnPlane = *(measurementsOnPlane.front());

    Eigen::Vector1d m_k(measurementOnPlane.getState().GetMatrixArray());
    Eigen::RowVector5d H_k(measurementOnPlane.getHMatrix()->getMatrix().GetMatrixArray());
    Eigen::Vector5d H_k_t = H_k.transpose();
    Eigen::Vector1d V_k(measurementOnPlane.getCov().GetMatrixArray());

    const Eigen::Vector5d& K_k = C_k_old * H_k_t * (V_k + H_k * C_k_old * H_k_t).inverse();

    C_k_old -= K_k * H_k * C_k_old;
    x_k_old += K_k * (m_k - H_k * x_k_old);

    measuredStateOnPlane.setState(TVectorD(5, x_k_old.data()));
    measuredStateOnPlane.setCov(TMatrixDSym(5, C_k_old.data()));

    Eigen::Vector1d residual = m_k - H_k * x_k_old;

    const double chi2 = (residual.transpose() * (V_k - H_k * C_k_old * H_k_t).inverse() * residual).value();
    return chi2;
  }

}
