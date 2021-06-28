/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Wouter Hulsbergen, Francesco Tenchini, Jo-Frederik Krohn  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <analysis/VertexFitting/TreeFitter/RecoKlong.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>

namespace TreeFitter {

  RecoKlong::RecoKlong(Belle2::Particle* particle, const ParticleBase* mother) :
    RecoParticle(particle, mother),
    m_dim(3),
    m_init(false),
    m_useEnergy(true),
    m_clusterPars(),
    m_covariance()
  {
    initParams() ;
  }

  ErrCode RecoKlong::initParticleWithMother(FitParams& fitparams)
  {
    const int posindexmother = mother()->posIndex();

    Eigen::Matrix<double, 1, 3> vertexToCluster = Eigen::Matrix<double, 1, 3>::Zero(1, 3);
    for (unsigned int i = 0; i < 3; ++i) {
      vertexToCluster(i) = m_clusterPars(i) - fitparams.getStateVector()(posindexmother + i);
    }

    const double distanceToMother = vertexToCluster.norm();
    const double energy =  m_clusterPars(3);
    const double energy2 = energy * energy;
    const double pdgMass2 = ParticleBase::pdgMass() * ParticleBase::pdgMass();
    const double absMom = -1 * std::sqrt(energy2 - pdgMass2);

    const int momindex = momIndex();

    for (unsigned int i = 0; i < 3; ++i) {
      //px = |p| dx/|dx|
      fitparams.getStateVector()(momindex + i) = absMom * vertexToCluster(i) / distanceToMother;
    }

    fitparams.getStateVector()(momindex + 3) = energy ;

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoKlong::initMotherlessParticle([[gnu::unused]] FitParams& fitparams)
  {
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoKlong::initCovariance(FitParams& fitparams) const
  {
    const int momindex = momIndex();
    const int posindex  = mother()->posIndex();

    const double factorE = 1000 * m_covariance(3, 3);
    const double factorX = 1000; // ~ 10cm error on initial vertex

    fitparams.getCovariance().block<4, 4>(momindex, momindex) =
      Eigen::Matrix<double, 4, 4>::Identity(4, 4) * factorE;

    fitparams.getCovariance().block<3, 3>(posindex, posindex) =
      Eigen::Matrix<double, 3, 3>::Identity(3, 3) * factorX;

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoKlong::initParams()
  {
    const Belle2::KLMCluster* cluster = particle()->getKLMCluster();

    const TVector3 cluster_pos = cluster->getClusterPosition();

    m_init = true;
    m_covariance =  Eigen::Matrix<double, 4, 4>::Zero(4, 4);

    TMatrixDSym cov7 = cluster->getError7x7();

    for (int row = 0; row < 3; ++row) {
      for (int col = 0; col < 3; ++col) {
        m_covariance(row, col) = cov7[row + 4][col + 4] ;
      }
    }

    /** currently the energy in KLM is calculated as n2dHits in cluster times 0.214 GeV
     *  at time of writing - 8.3.18 - the KLMCluster returns 0 for the E covariance
     * */
    if (0 == m_covariance(3, 3)) {m_covariance(3, 3) = .214;}

    m_clusterPars(0) = cluster_pos.X();
    m_clusterPars(1) = cluster_pos.Y();
    m_clusterPars(2) = cluster_pos.Z();
    m_clusterPars(3) = sqrt(particle()->getPDGMass() * particle()->getPDGMass() + cluster->getMomentumMag() *
                            cluster->getMomentumMag());

    auto p_vec = particle()->getMomentum();
    // find highest momentum, eliminate dim with highest mom
    if ((std::abs(p_vec(0)) >= std::abs(p_vec(1))) && (std::abs(p_vec(0)) >= std::abs(p_vec(2)))) {
      m_i1 = 0; m_i2 = 1; m_i3 = 2;
    } else if ((std::abs(p_vec(1)) >= std::abs(p_vec(0))) && (std::abs(p_vec(1)) >= std::abs(p_vec(2)))) {
      m_i1 = 1; m_i2 = 0; m_i3 = 2;
    } else if ((std::abs(p_vec(2)) >= std::abs(p_vec(1))) && (std::abs(p_vec(2)) >= std::abs(p_vec(0)))) {
      m_i1 = 2; m_i2 = 1; m_i3 = 0;
    } else {
      B2ERROR("Could not estimate highest momentum for Klong constraint. Aborting this fit.\n px: "
              << p_vec(0) << " py: " << p_vec(1) << " pz: " << p_vec(2) << " calculated from Ec: " << m_clusterPars(3));
      return ErrCode(ErrCode::Status::photondimerror);
    }

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoKlong::projectRecoConstraint(const FitParams& fitparams, Projection& p) const
  {
    const int momindex  = momIndex() ;
    const int posindex  = mother()->posIndex();

    const Eigen::Matrix<double, 1, 3> x_vertex = fitparams.getStateVector().segment(posindex, 3);
    const Eigen::Matrix<double, 1, 3> p_vec = fitparams.getStateVector().segment(momindex, 3);

    if (0 == p_vec[m_i1]) { return ErrCode(ErrCode::Status::klongdimerror); }

    // p_vec[m_i1] must not be 0
    const double elim = (m_clusterPars[m_i1] - x_vertex[m_i1]) / p_vec[m_i1];
    const double mom = p_vec.norm();
    const double energy = fitparams.getStateVector()(momindex + 3);

    // r'
    Eigen::Matrix<double, 3, 1> residual3 = Eigen::Matrix<double, 3, 1>::Zero(3, 1);
    residual3(0) = m_clusterPars[m_i2] - x_vertex[m_i2] - p_vec[m_i2] * elim;
    residual3(1) = m_clusterPars[m_i3] - x_vertex[m_i3] - p_vec[m_i3] * elim;
    residual3(2) = m_clusterPars[3] - energy;

    Eigen::Matrix<double, 3, 4> P = Eigen::Matrix<double, 3, 4>::Zero(3, 4);

    // dr'/dm | m:={xc,yc,zc,Ec} the measured quantities
    P(0, m_i2) = 1;
    P(0, m_i1) = - p_vec[m_i2] / p_vec[m_i1];

    P(1, m_i3) = 1;
    P(1, m_i1) = - p_vec[m_i3] / p_vec[m_i1];
    P(2, 3) = 1; // dE/dEc

    p.getResiduals().segment(0, 3) = residual3;

    p.getV() = P * m_covariance.selfadjointView<Eigen::Lower>() * P.transpose();

    // dr'/dm  | m:={x,y,z,px,py,pz,E}
    // x := x_vertex (decay vertex of mother)
    p.getH()(0, posindex + m_i1) =  p_vec[m_i2] / p_vec[m_i1];
    p.getH()(0, posindex + m_i2) = -1.0;
    p.getH()(0, posindex + m_i3) = 0;

    p.getH()(1, posindex + m_i1) =  p_vec[m_i3] / p_vec[m_i1];
    p.getH()(1, posindex + m_i2) = 0;
    p.getH()(1, posindex + m_i3) = -1.0;

    p.getH()(0, momindex + m_i1) = p_vec[m_i2] * elim / p_vec[m_i1];
    p.getH()(0, momindex + m_i2) = -1. * elim;
    p.getH()(0, momindex + m_i3) = 0;

    p.getH()(1, momindex + m_i1) = p_vec[m_i3] * elim / p_vec[m_i1];;
    p.getH()(1, momindex + m_i2) = 0;
    p.getH()(1, momindex + m_i3) = -1. * elim;

    p.getH()(2, momindex + m_i1) = -1. * p_vec[m_i1] / mom;
    p.getH()(2, momindex + m_i2) = -1. * p_vec[m_i2] / mom;
    p.getH()(2, momindex + m_i3) = -1. * p_vec[m_i3] / mom;
    p.getH()(2, momindex + 3) = -1;

    return ErrCode(ErrCode::Status::success);
  }

}


