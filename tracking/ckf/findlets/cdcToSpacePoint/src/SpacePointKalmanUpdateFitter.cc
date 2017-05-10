/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointKalmanUpdateFitter.h>
#include <svd/reconstruction/SVDRecoHit.h>

#include <Eigen/Dense>

namespace Eigen {
  using Vector5d = Matrix<double, 5, 1>;
  using RowVector5d = Matrix<double, 1, 5>;
  using Vector1d = Matrix<double, 1, 1>;
  using Matrix5d = Matrix<double, 5, 5>;
}

using namespace Belle2;
using namespace TrackFindingCDC;

Weight SpacePointKalmanUpdateFitter::operator()(CKFCDCToVXDStateObject& currentState)
{
  B2ASSERT("Encountered invalid state", not currentState.isFitted() and currentState.isAdvanced());

  const SpacePoint* spacePoint = currentState.getHit();

  if (not spacePoint) {
    // If we do not have a space point, we do not need to do anything here.
    currentState.setFitted();
    return 1;
  }

  genfit::MeasuredStateOnPlane& measuredStateOnPlane = currentState.getMeasuredStateOnPlane();
  double chi2 = 0;

  // We will change the state x_k, the covariance C_k and the chi2
  Eigen::Vector5d x_k_old(measuredStateOnPlane.getState().GetMatrixArray());
  Eigen::Matrix5d C_k_old(measuredStateOnPlane.getCov().GetMatrixArray());

  // Loop over the two clusters and extract the change for x_k and C_k.
  for (const SVDCluster& relatedCluster : spacePoint->getRelationsTo<SVDCluster>()) {
    SVDRecoHit clusterMeasurement(&relatedCluster);
    // Important: measuredStateOnPlane must already be extrapolated to the correct plane.
    // Only the plane and the rep are accessed (but the rep has no meaningful members).
    const std::vector<genfit::MeasurementOnPlane*> measurementsOnPlane = clusterMeasurement.constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("There should be exactly one measurement on plane", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane& measurementOnPlane = *(measurementsOnPlane.front());

    Eigen::Vector1d m_k(measurementOnPlane.getState().GetMatrixArray());
    Eigen::RowVector5d H_k(measurementOnPlane.getHMatrix()->getMatrix().GetMatrixArray());
    Eigen::Vector5d H_k_t = H_k.transpose();
    Eigen::Vector1d V_k(measurementOnPlane.getCov().GetMatrixArray());

    const Eigen::Vector5d& K_k = C_k_old * H_k_t * (V_k + H_k * C_k_old * H_k_t).inverse();

    B2DEBUG(200, "C_k_old " << C_k_old);
    B2DEBUG(200, "H_k " << H_k);
    B2DEBUG(200, "m_k " << m_k);
    B2DEBUG(200, "V_k " << V_k);
    B2DEBUG(200, "x_k_old " << x_k_old);
    B2DEBUG(200, "K_k " << K_k);

    C_k_old -= K_k * H_k * C_k_old;
    x_k_old += K_k * (m_k - H_k * x_k_old);

    Eigen::Vector1d residual = m_k - H_k * x_k_old;

    chi2 += (residual.transpose() * (V_k - H_k * C_k_old * H_k_t).inverse() * residual).value();

    B2DEBUG(200, "C_k_old " << C_k_old);
    B2DEBUG(200, "x_k_old " << x_k_old);
    B2DEBUG(100, "chi2 " << chi2);
  }

  measuredStateOnPlane.setState(TVectorD(5, x_k_old.data()));
  measuredStateOnPlane.setCov(TMatrixDSym(5, C_k_old.data()));
  currentState.setChi2(chi2);

  currentState.setFitted();
  return 1;
}
