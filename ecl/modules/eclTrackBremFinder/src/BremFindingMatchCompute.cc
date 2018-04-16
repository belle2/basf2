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
  auto fitted_dir = fitted_state.getDir();

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

  const double err_phi = (py / square_perp) * err_px +
                         (pz / square_perp) * err_py;
  const double err_theta = (px * pz) / (square_sum * perp) * err_px +
                           (py * pz) / (square_sum * perp) * err_py +
                           (perp / square_sum) * err_pz;


  const auto hit_theta = fitted_mom.Theta();
  const auto hit_phi = fitted_mom.Phi();

  PhiAngle clusterPhi = PhiAngle(0, 0);
  ThetaAngle clusterTheta = ThetaAngle(0, 0);

  clusterPhi = PhiAngle(clusterPosition.Phi(), m_eclCluster.getUncertaintyPhi());
  clusterTheta = ThetaAngle(clusterPosition.Theta(), m_eclCluster.getUncertaintyTheta());

  PhiAngle hitPhi(hit_phi, err_phi);
  ThetaAngle hitTheta(hit_theta, err_theta);

  if (clusterPhi.containsIn(hitPhi, m_clusterAcceptanceFactor) &&
      clusterTheta.containsIn(hitTheta, m_clusterAcceptanceFactor)) {

    TVector3 hitV;
    hitV.SetMagThetaPhi(1.0f, hitTheta.getAngle(), hitPhi.getAngle());
    TVector3 clusterV;
    clusterV.SetMagThetaPhi(1.0f, clusterTheta.getAngle(), clusterPhi.getAngle());

    auto distV = hitV - clusterV;

    m_distanceHitCluster = distV.Mag();

    return (true);

    // todo: can use weight here to signal assigment confidence

  } else {
    return (false);
  }
}
