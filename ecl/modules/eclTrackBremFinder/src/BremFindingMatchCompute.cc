/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclTrackBremFinder/BremFindingMatchCompute.h>

/* Basf2 headers. */
#include <framework/geometry/VectorUtil.h>
#include <framework/utilities/Angle.h>
#include <mdst/dataobjects/ECLCluster.h>

/* Genfit headers. */
#include <genfit/MeasuredStateOnPlane.h>

/* ROOT headers. */
#include <Math/Vector3D.h>

using namespace std;
using namespace Belle2;

bool BremFindingMatchCompute::isMatch()
{
  auto fitted_state = m_measuredStateOnPlane;

  ROOT::Math::XYZVector clusterPosition = m_eclCluster->getClusterPosition();

  TVector3 position = fitted_state.getPos();
  ROOT::Math::XYZVector positionDifference(
    clusterPosition.X() - position.X(),
    clusterPosition.Y() - position.Y(),
    clusterPosition.Z() - position.Z());
  TVector3 momentum = fitted_state.getMom();
  ROOT::Math::XYZVector fitted_mom(momentum.X(), momentum.Y(), momentum.Z());

  auto cov = fitted_state.get6DCov();
  const double err_px = cov[3][3];
  const double err_py = cov[4][4];
  const double err_pz = cov[5][5];
  const double px = fitted_mom.X();
  const double py = fitted_mom.Y();
  const double pz = fitted_mom.Z();

  const double square_perp =  px * px + py * py ;
  const double perp = std::sqrt(square_perp);
  const double square_sum = square_perp + pz * pz;

  const double err_phi = std::sqrt(pow((py / square_perp), 2) * err_px + pow((px / square_perp), 2) * err_py +
                                   (py / square_perp) * (px / square_perp) * cov[3][4]);
  const double err_theta = std::sqrt(pow(((px * pz) / (square_sum * perp)), 2) * err_px +
                                     pow(((py * pz) / (square_sum * perp)), 2) * err_py +
                                     pow((perp / square_sum), 2) * err_pz +
                                     ((px * pz) / (square_sum * perp)) * ((py * pz) / (square_sum * perp)) * cov[3][4] +
                                     ((px * pz) / (square_sum * perp)) * (perp / square_sum) * cov[3][5] +
                                     ((py * pz) / (square_sum * perp)) * (perp / square_sum) * cov[4][5]);

  PhiAngle   hitPhi(fitted_mom.Phi(), err_phi);
  ThetaAngle hitTheta(fitted_mom.Theta(), err_theta);

  PhiAngle clusterPhi(positionDifference.Phi(), m_eclCluster->getUncertaintyPhi());
  ThetaAngle clusterTheta(positionDifference.Theta(), m_eclCluster->getUncertaintyTheta());

  if (clusterPhi.containsIn(hitPhi, m_clusterAcceptanceFactor) &&
      clusterTheta.containsIn(hitTheta, m_clusterAcceptanceFactor)) {

    ROOT::Math::XYZVector hitV;
    VectorUtil::setMagThetaPhi(
      hitV, 1.0, hitTheta.getAngle(), hitPhi.getAngle());
    ROOT::Math::XYZVector clusterV;
    VectorUtil::setMagThetaPhi(
      clusterV, 1.0, clusterTheta.getAngle(), clusterPhi.getAngle());

    ROOT::Math::XYZVector distV = hitV - clusterV;

    m_distanceHitCluster = distV.R();

    // set the effective acceptance factor
    double deltaTheta = abs(hitV.Y() - clusterV.Y());
    m_effAcceptanceFactor = deltaTheta / (err_theta + m_eclCluster->getUncertaintyTheta());
    double deltaPhi = abs(hitV.Z() - clusterV.Z());
    double effFactor = deltaPhi / (err_phi + m_eclCluster->getUncertaintyPhi());
    if (effFactor > m_effAcceptanceFactor) {
      m_effAcceptanceFactor = effFactor;
    }

    return (true);

    // todo: can use weight here to signal assigment confidence

  } else {
    return (false);
  }
}
