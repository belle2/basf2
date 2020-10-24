/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 * Contributors: Thomas Hauth, Patrick Ecker                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclTrackBremFinder/BremFindingMatchCompute.h>

//Framework
#include <framework/utilities/Angle.h>

//MDST
#include <mdst/dataobjects/ECLCluster.h>

//genfit
#include <genfit/MeasuredStateOnPlane.h>

using namespace std;
using namespace Belle2;

bool BremFindingMatchCompute::isMatch()
{
  auto fitted_state = m_measuredStateOnPlane;

  auto clusterPosition = m_eclCluster.getClusterPosition();

  auto fitted_pos = fitted_state.getPos();
  auto fitted_mom = fitted_state.getMom();

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


  const auto hit_theta = fitted_mom.Theta();
  double hit_phi       = fitted_mom.Phi();
  if (hit_phi < 0) hit_phi += TMath::TwoPi();

  PhiAngle   hitPhi(hit_phi, err_phi);
  ThetaAngle hitTheta(hit_theta, err_theta);

  PhiAngle clusterPhi(0, 0);

  if ((clusterPosition - fitted_pos).Phi() >= 0) {
    clusterPhi = PhiAngle((clusterPosition - fitted_pos).Phi(), m_eclCluster.getUncertaintyPhi());
  } else {
    clusterPhi = PhiAngle((clusterPosition - fitted_pos).Phi() + TMath::TwoPi(), m_eclCluster.getUncertaintyPhi());
  }
  ThetaAngle clusterTheta = ThetaAngle((clusterPosition - fitted_pos).Theta(), m_eclCluster.getUncertaintyTheta());

  if (clusterPhi.containsIn(hitPhi, m_clusterAcceptanceFactor) &&
      clusterTheta.containsIn(hitTheta, m_clusterAcceptanceFactor)) {

    TVector3 hitV;
    hitV.SetMagThetaPhi(1.0f, hitTheta.getAngle(), hitPhi.getAngle());
    TVector3 clusterV;
    clusterV.SetMagThetaPhi(1.0f, clusterTheta.getAngle(), clusterPhi.getAngle());

    auto distV = hitV - clusterV;

    m_distanceHitCluster = distV.Mag();

    // set the effective acceptance factor
    double deltaTheta = abs(hitV.Y() - clusterV.Y());
    m_effAcceptanceFactor = deltaTheta / (err_theta + m_eclCluster.getUncertaintyTheta());
    double deltaPhi = abs(hitV.Z() - clusterV.Z());
    double effFactor = deltaPhi / (err_phi + m_eclCluster.getUncertaintyPhi());
    if (effFactor > m_effAcceptanceFactor) {
      m_effAcceptanceFactor = effFactor;
    }

    return (true);

    // todo: can use weight here to signal assigment confidence

  } else {
    return (false);
  }
}
