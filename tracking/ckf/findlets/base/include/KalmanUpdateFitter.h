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

#include <tracking/ckf/utilities/EigenHelper.h>

#include <Eigen/Dense>

namespace Eigen {
  /// A typical state vector
  using Vector5d = Matrix<double, 5, 1>;
  /// A typical cov matrix
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
   *
   * We use some common notation in this class (mostly copied from the Kalman paper by Frühwirth):
   * * x_k is the state of the track (5 dimensional) as plane k (the plane of the hit) in helix coordinates
   * * C_k is its covariance matrix at the same plane - also in helix coordinates
   * * m_k is the measured state of the hit - in hit coordinates
   * * H_k is the matrix connecting hit coordinates with helix coordinates
   * * V_k is the covariance matrix of m_k
   */
  class KalmanUpdateFitter : public TrackFindingCDC::CompositeProcessingSignalListener {
  public:
    /// Main function: update the parameters stored in the mSoP of the state using the hit related to this state.
    template <class AState>
    TrackFindingCDC::Weight operator()(AState& currentState) const;

    /**
     * Reusable function to do the kalman update of a mSoP with the information in the hit.
     * Wrapper around kalmanStepImplementation using the correct number of dimensions
     */
    template <class ARecoHit>
    double kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const;

    /// Currently, no parameters are exported.
    void exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                          const std::string& prefix __attribute__((unused)))
    {}

    /// Helper function to calculate the chi2 out of the given matrices/vectors. Mostly useful internally.
    template <unsigned int Dimension>
    double calculateChi2(const Eigen::Vector5d& x_k, const Eigen::Matrix5d& C_k,
                         const Eigen::Matrix<double, Dimension, 1>& m_k,
                         const Eigen::Matrix<double, Dimension, 5>& H_k,
                         const Eigen::Matrix<double, Dimension, Dimension>& V_k) const;

    /// Helper function to calculate the chi2 of the given state (which must be on the same plane as the hit) with the hit.
    template <class ARecoHit, unsigned int Dimension>
    double calculateChi2(const genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const;

    template <class ARecoHit, unsigned int Dimension>
    double calculateResidualDistance(const genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const;

  private:
    /// Implementation of the kalman update step for a generic hit class.
    template <class ARecoHit, unsigned int Dimension>
    double kalmanStepImplementation(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const;

    /// Extract all needed matrices from the given state
    std::pair<Eigen::Vector5d, Eigen::Matrix5d> extractMatricesFromState(const genfit::MeasuredStateOnPlane& measuredStateOnPlane)
    const
    {
      const Eigen::Vector5d& x_k = convertToEigen<5>(measuredStateOnPlane.getState());
      const Eigen::Matrix5d& C_k = convertToEigen<5, 5>(measuredStateOnPlane.getCov());

      return std::make_pair(x_k, C_k);
    }

    /// Extract all needed matrices from the given reco hit
    template <class ARecoHit, unsigned int Dimension>
    std::tuple<Eigen::Matrix<double, Dimension, 1>, Eigen::Matrix<double, Dimension, 5>, Eigen::Matrix<double, Dimension, Dimension>>
        extractMatricesFromHit(const ARecoHit& recoHit, const genfit::MeasuredStateOnPlane& measuredStateOnPlane) const
    {
      // Important: measuredStateOnPlane must already be extrapolated to the correct plane.
      // Only the plane and the rep are accessed (but the rep has no meaningful members).
      const std::vector<genfit::MeasurementOnPlane*> measurementsOnPlane = recoHit.constructMeasurementsOnPlane(
            measuredStateOnPlane);
      B2ASSERT("There should be exactly one measurement on plane", measurementsOnPlane.size() == 1);
      const genfit::MeasurementOnPlane& measurementOnPlane = *(measurementsOnPlane.front());

      const Eigen::Matrix<double, Dimension, 1>& m_k = convertToEigen<Dimension>(measurementOnPlane.getState());
      const Eigen::Matrix<double, Dimension, 5>& H_k = convertToEigen<Dimension, 5>(measurementOnPlane.getHMatrix()->getMatrix());
      const Eigen::Matrix<double, Dimension, Dimension>& V_k = convertToEigen<Dimension, Dimension>(measurementOnPlane.getCov());

      delete measurementsOnPlane.front();

      return std::make_tuple(m_k, H_k, V_k);
    };
  };

  template <class AState>
  TrackFindingCDC::Weight KalmanUpdateFitter::operator()(AState& currentState) const
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

  template <unsigned int Dimension>
  double KalmanUpdateFitter::calculateChi2(const Eigen::Vector5d& x_k, const Eigen::Matrix5d& C_k,
                                           const Eigen::Matrix<double, Dimension, 1>& m_k,
                                           const Eigen::Matrix<double, Dimension, 5>& H_k,
                                           const Eigen::Matrix<double, Dimension, Dimension>& V_k) const
  {
    const Eigen::Matrix<double, Dimension, 1>& residual = m_k - H_k * x_k;
    const double chi2 = (residual.transpose() * (V_k - H_k * C_k * H_k.transpose()).inverse() * residual).value();
    return chi2;
  }

  template <class ARecoHit, unsigned int Dimension>
  double KalmanUpdateFitter::calculateChi2(const genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const
  {
    Eigen::Vector5d x_k;
    Eigen::Matrix5d C_k;
    std::tie(x_k, C_k) = extractMatricesFromState(measuredStateOnPlane);

    Eigen::Matrix<double, Dimension, 1> m_k;
    Eigen::Matrix<double, Dimension, 5> H_k;
    Eigen::Matrix<double, Dimension, Dimension> V_k;
    std::tie(m_k, H_k, V_k) = extractMatricesFromHit<ARecoHit, Dimension>(recoHit, measuredStateOnPlane);

    return calculateChi2<Dimension>(x_k, C_k, m_k, H_k, V_k);
  }

  template <class ARecoHit, unsigned int Dimension>
  double KalmanUpdateFitter::calculateResidualDistance(const genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                                       ARecoHit& recoHit) const
  {
    Eigen::Vector5d x_k;
    Eigen::Matrix5d C_k;
    std::tie(x_k, C_k) = extractMatricesFromState(measuredStateOnPlane);

    Eigen::Matrix<double, Dimension, 1> m_k;
    Eigen::Matrix<double, Dimension, 5> H_k;
    Eigen::Matrix<double, Dimension, Dimension> V_k;
    std::tie(m_k, H_k, V_k) = extractMatricesFromHit<ARecoHit, Dimension>(recoHit, measuredStateOnPlane);

    const Eigen::Matrix<double, Dimension, 1>& residual = m_k - H_k * x_k;
    return residual.norm();
  };

  template <class ARecoHit, unsigned int Dimension>
  double KalmanUpdateFitter::kalmanStepImplementation(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const
  {
    Eigen::Vector5d x_k;
    Eigen::Matrix5d C_k;
    std::tie(x_k, C_k) = extractMatricesFromState(measuredStateOnPlane);

    Eigen::Matrix<double, Dimension, 1> m_k;
    Eigen::Matrix<double, Dimension, 5> H_k;
    Eigen::Matrix<double, Dimension, Dimension> V_k;
    std::tie(m_k, H_k, V_k) = extractMatricesFromHit<ARecoHit, Dimension>(recoHit, measuredStateOnPlane);

    // This now is the real "update" step, where we update the X_k and the C_k.
    const Eigen::Matrix<double, 5, Dimension>& K_k = C_k * H_k.transpose() * (V_k + H_k * C_k * H_k.transpose()).inverse();
    C_k -= K_k * H_k * C_k;
    x_k += K_k * (m_k - H_k * x_k);

    measuredStateOnPlane.setState(TVectorD(5, x_k.data()));
    measuredStateOnPlane.setCov(TMatrixDSym(5, C_k.data()));

    // We return the new chi2
    const double chi2 = calculateChi2<Dimension>(x_k, C_k, m_k, H_k, V_k);
    return chi2;
  }

}
