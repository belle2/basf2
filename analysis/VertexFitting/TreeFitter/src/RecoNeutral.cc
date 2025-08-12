/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <analysis/VertexFitting/TreeFitter/RecoNeutral.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>

#include <framework/gearbox/Const.h>

namespace TreeFitter {

  RecoNeutral::RecoNeutral(Belle2::Particle* particle, const ParticleBase* mother) : RecoParticle(particle, mother),
    m_dim(3),
    m_init(false),
    m_clusterPars(),
    m_covariance(),
    m_momentumScalingFactor(particle->getEffectiveMomentumScale()),
    m_mass(particle->getPDGMass()),
    m_particleSource(particle->getParticleSource())
  {
    initParams();
  }

  ErrCode RecoNeutral::initParticleWithMother(FitParams& fitparams)
  {
    const int posindexmother = mother()->posIndex();

    Eigen::Matrix<double, 1, 3> vertexToCluster = Eigen::Matrix<double, 1, 3>::Zero(1, 3);
    for (unsigned int i = 0; i < 3; ++i) {
      vertexToCluster(i) = m_clusterPars(i) - fitparams.getStateVector()(posindexmother + i);
    }

    const double distanceToMother = vertexToCluster.norm();
    const double energy = m_momentumScalingFactor * m_clusterPars(3); // apply scaling factor to correct energy bias
    const double absMom = std::sqrt(energy * energy - m_mass * m_mass);

    const int momindex = momIndex();

    for (unsigned int i = 0; i < 3; ++i) {
      fitparams.getStateVector()(momindex + i) = absMom * vertexToCluster(i) / distanceToMother;
    }

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoNeutral::initMotherlessParticle([[gnu::unused]] FitParams& fitparams)
  {
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoNeutral::initCovariance(FitParams& fitparams) const
  {
    const int momindex = momIndex();
    const int posindex = mother()->posIndex();

    const double factorE = 1000 * m_covariance(3, 3);
    const double factorX = 1000; // ~ 10cm error on initial vertex

    fitparams.getCovariance().block<4, 4>(momindex, momindex) =
      Eigen::Matrix<double, 4, 4>::Identity(4, 4) * factorE;

    fitparams.getCovariance().block<3, 3>(posindex, posindex) =
      Eigen::Matrix<double, 3, 3>::Identity(3, 3) * factorX;

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoNeutral::initParams()
  {
    ROOT::Math::XYZVector clusterCenter;
    double energy;
    m_covariance = Eigen::Matrix<double, 4, 4>::Zero(4, 4);
    if (m_particleSource == Belle2::Particle::EParticleSourceObject::c_KLMCluster) {
      const Belle2::KLMCluster* cluster = particle()->getKLMCluster();
      clusterCenter = cluster->getClusterPosition();
      const double momentum = cluster->getMomentumMag();
      energy = sqrt(m_mass * m_mass + momentum * momentum);

      TMatrixDSym cov7 = cluster->getError7x7();
      for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
          m_covariance(row, col) = cov7[row + 4][col + 4] ;
        }
      }

      /** currently the energy in KLM is calculated as n2dHits in cluster times 0.214 GeV
       *  at time of writing - 8.3.18 - the KLMCluster returns 0 for the E covariance
       */
      if (0 == m_covariance(3, 3)) { m_covariance(3, 3) = .214; }
    } else if (m_particleSource == Belle2::Particle::EParticleSourceObject::c_ECLCluster) {
      const Belle2::ECLCluster* cluster = particle()->getECLCluster();
      clusterCenter = cluster->getClusterPosition();
      const Belle2::ECLCluster::EHypothesisBit clusterhypo = particle()->getECLClusterEHypothesisBit();
      energy = cluster->getEnergy(clusterhypo);

      Belle2::ClusterUtils C;
      TMatrixDSym cov_EPhiTheta = C.GetCovarianceMatrix3x3FromCluster(cluster);

      Eigen::Matrix<double, 2, 2> covPhiTheta = Eigen::Matrix<double, 2, 2>::Zero(2, 2);

      for (int row = 0; row < 2; ++row) {
        // we go through all elements here instead of selfadjoint view later
        for (int col = 0; col < 2; ++col) {
          covPhiTheta(row, col) = cov_EPhiTheta[row + 1][col + 1];
        }
      }

      // the in going x-E correlations are 0 so we don't fill them
      const double R = cluster->getR();
      const double theta = cluster->getPhi();
      const double phi = cluster->getTheta();

      const double st = std::sin(theta);
      const double ct = std::cos(theta);
      const double sp = std::sin(phi);
      const double cp = std::cos(phi);

      Eigen::Matrix<double, 2, 3> polarToCartesian = Eigen::Matrix<double, 2, 3>::Zero(2, 3);

      // polarToCartesian({phi,theta} -> {x,y,z} )
      polarToCartesian(0, 0) = -1. * R * st * sp; // dx/dphi
      polarToCartesian(0, 1) = R * st * cp;       // dy/dphi
      polarToCartesian(0, 2) = 0;                 // dz/dphi

      polarToCartesian(1, 0) = R * ct * cp;  // dx/dtheta
      polarToCartesian(1, 1) = R * ct * sp;  // dy/dtheta
      polarToCartesian(1, 2) = -1. * R * st; // dz/dtheta

      m_covariance.block<3, 3>(0, 0) = polarToCartesian.transpose() * covPhiTheta * polarToCartesian;

      m_covariance(3, 3) = cov_EPhiTheta[0][0];
    }

    m_init = true;

    m_clusterPars(0) = clusterCenter.X();
    m_clusterPars(1) = clusterCenter.Y();
    m_clusterPars(2) = clusterCenter.Z();
    m_clusterPars(3) = energy;

    auto p_vec = particle()->getMomentum();
    // find highest momentum, eliminate dim with highest mom
    if ((std::abs(p_vec.X()) >= std::abs(p_vec.Y())) && (std::abs(p_vec.X()) >= std::abs(p_vec.Z()))) {
      m_i1 = 0;
      m_i2 = 1;
      m_i3 = 2;
    } else if ((std::abs(p_vec.Y()) >= std::abs(p_vec.X())) && (std::abs(p_vec.Y()) >= std::abs(p_vec.Z()))) {
      m_i1 = 1;
      m_i2 = 0;
      m_i3 = 2;
    } else if ((std::abs(p_vec.Z()) >= std::abs(p_vec.Y())) && (std::abs(p_vec.Z()) >= std::abs(p_vec.X()))) {
      m_i1 = 2;
      m_i2 = 1;
      m_i3 = 0;
    } else {
      B2ERROR("Could not estimate highest momentum for neutral particle constraint. Aborting this fit.\n px: "
              << p_vec.X() << " py: " << p_vec.Y() << " pz: " << p_vec.Z() << " calculated from Ec: " << m_clusterPars(3));
      return ErrCode(ErrCode::Status::photondimerror);
    }

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoNeutral::projectRecoConstraint(const FitParams& fitparams, Projection& p) const
  {
    const int momindex = momIndex();
    const int posindex = mother()->posIndex();
    /**
     * m : decay vertex mother
     * p : momentum vector
     * c : position cluster
     * so:
     * m + p = c
     * thus (tau converts p to the correct units):
     * 0 = c - m - tau * p
     * we have 3 geometric equations and eliminate tau using the dimension with the highest momentum
     * (because we have to divide by that momentum)
     * only downside is we have to figure out which dimension this is
     * the 4th equation is the energy which we keep as:
     * 0 = E - |p|
     */

    const Eigen::Matrix<double, 1, 3> x_vertex = fitparams.getStateVector().segment(posindex, 3);
    const Eigen::Matrix<double, 1, 3> p_vec = fitparams.getStateVector().segment(momindex, 3);

    if (0 == p_vec[m_i1]) { return ErrCode(ErrCode::photondimerror); }

    // p_vec[m_i1] must not be 0
    const double elim = (m_clusterPars[m_i1] - x_vertex[m_i1]) / p_vec[m_i1];
    const double mom = p_vec.norm();
    const double energy = std::sqrt(mom * mom + m_mass * m_mass);

    // r'
    Eigen::Matrix<double, 3, 1> residual3 = Eigen::Matrix<double, 3, 1>::Zero(3, 1);
    residual3(0) = m_clusterPars[m_i2] - x_vertex[m_i2] - p_vec[m_i2] * elim;
    residual3(1) = m_clusterPars[m_i3] - x_vertex[m_i3] - p_vec[m_i3] * elim;
    residual3(2) = m_momentumScalingFactor * m_clusterPars[3] - energy; // scale measured energy by scaling factor

    // dr'/dm | m:={xc,yc,zc,Ec} the measured quantities
    Eigen::Matrix<double, 3, 4> P = Eigen::Matrix<double, 3, 4>::Zero(3, 4);
    // deriving by the cluster pars
    P(0, m_i2) = 1;
    P(0, m_i1) = -p_vec[m_i2] / p_vec[m_i1];

    P(1, m_i3) = 1;
    P(1, m_i1) = -p_vec[m_i3] / p_vec[m_i1];
    P(2, 3) = 1; // dE/dEc

    p.getResiduals().segment(0, 3) = residual3;

    p.getV() = P * m_covariance.selfadjointView<Eigen::Lower>() * P.transpose();

    // dr'/dm  | m:={x,y,z,px,py,pz,E}
    // x := x_vertex (decay vertex of mother)
    p.getH()(0, posindex + m_i1) = p_vec[m_i2] / p_vec[m_i1];
    p.getH()(0, posindex + m_i2) = -1.0;
    p.getH()(0, posindex + m_i3) = 0;

    p.getH()(1, posindex + m_i1) = p_vec[m_i3] / p_vec[m_i1];
    p.getH()(1, posindex + m_i2) = 0;
    p.getH()(1, posindex + m_i3) = -1.0;

    // elim already divided by p_vec[m_i1]
    p.getH()(0, momindex + m_i1) = p_vec[m_i2] * elim / p_vec[m_i1];
    p.getH()(0, momindex + m_i2) = -1. * elim;
    p.getH()(0, momindex + m_i3) = 0;

    p.getH()(1, momindex + m_i1) = p_vec[m_i3] * elim / p_vec[m_i1];
    p.getH()(1, momindex + m_i2) = 0;
    p.getH()(1, momindex + m_i3) = -1. * elim;

    p.getH()(2, momindex + m_i1) = -1. * p_vec[m_i1] / mom;
    p.getH()(2, momindex + m_i2) = -1. * p_vec[m_i2] / mom;
    p.getH()(2, momindex + m_i3) = -1. * p_vec[m_i3] / mom;

    return ErrCode(ErrCode::Status::success);
  }

}
