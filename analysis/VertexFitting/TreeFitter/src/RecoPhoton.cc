/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Creates photons and other neutrals from cluster objects. Currently only tested for photons.

//#include <stdio.h>
#include <framework/logging/Logger.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <analysis/VertexFitting/TreeFitter/RecoPhoton.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>

#include <framework/gearbox/Const.h>

namespace TreeFitter {
  extern int vtxverbose ;

  RecoPhoton::RecoPhoton(Belle2::Particle* particle, const ParticleBase* mother)
    : RecoParticle(particle, mother),
      m_init(false),
      m_useEnergy(useEnergy(*particle)),
      m_params(),//use params for dim if klongs come into the game
      m_covariance()
  {
    initParams() ;
  }

  RecoPhoton::~RecoPhoton()
  {
  }

  ErrCode RecoPhoton::initParticleWithMother(FitParams* fitparams)
  {
    // calculate the direction
    int posindexmother = mother()->posIndex();

    Eigen::Matrix<double, 1, 3> vertexToCluster = Eigen::Matrix<double, 1, 3>::Zero(1, 3);
    for (unsigned int i = 0; i < 3; ++i) {
      vertexToCluster(i) = m_params(i) - fitparams->getStateVector()(posindexmother + i);
    }

    double distanceToMother = vertexToCluster.norm();
    // get the energy
    double energy = m_useEnergy ? m_params(3) : particle()->getEnergy();
    int momindex = momIndex();
    for (unsigned int i = 0; i < 3; ++i) {
      fitparams->getStateVector()(momindex + i) =  energy * vertexToCluster(i) / distanceToMother;
    }

    fitparams->getStateVector()(momindex + 3) =  energy;

    return ErrCode::success;
  }

  ErrCode RecoPhoton::initMotherlessParticle([[gnu::unused]] FitParams* fitparams)
  {
    return ErrCode::success;
  }

  //FT: this is needed once Klongs become involved
  bool RecoPhoton::useEnergy(Belle2::Particle& particle)
  {
    bool rc = true ;
    int pdg = particle.getPDGCode();
    if (pdg &&
        Belle2::Const::ParticleType(pdg) != Belle2::Const::photon && //   pdg != 22 &&
        Belle2::Const::ParticleType(pdg) != Belle2::Const::pi0) { //   pdg != 111){
      rc = false ;
    }
    //    B2DEBUG(80, "RecoPhoton::useEnergy = " << rc << " , pdg = " << pdg);
    return rc ;
  }

  ErrCode RecoPhoton::initCovariance(FitParams* fitparams) const
  {
    int momindex = momIndex() ;
    double varEnergy =  m_useEnergy ? m_covariance(3, 3) : 1;
    const double factor = 1000;

    for (int row = 0; row < 4; ++row) {
      fitparams->getCovariance()(momindex + row, momindex + row) = factor * varEnergy;
    }
    return ErrCode();
  }

  ErrCode RecoPhoton::initParams()
  {
    const Belle2::ECLCluster* recoCalo = particle()->getECLCluster();
    TVector3 centroid = recoCalo->getClusterPosition();
    double energy = recoCalo->getEnergy();
    m_init = true;
    // This returns the covariance matrix assuming the photons comes from the nominal IP
    Belle2::ClusterUtils C;
    TMatrixDSym cov_pE = C.GetCovarianceMatrix4x4FromCluster(
                           recoCalo);//FT: error on xyz is extracted from error on p (sort of backwards but ok)

    for (int row = 0; row < 4; ++row) {
      for (int col = 0; col <= row; ++col) {
        m_covariance(row, col) = cov_pE[row][col];
      }
    }
    m_params(0) = centroid.X();
    m_params(1) = centroid.Y();
    m_params(2) = centroid.Z();
    if (m_useEnergy) {
      m_params(3) = energy;
    } else {
      m_params(3) = 0; //JFK: does this make sense 2017-10-25
    }
    return ErrCode::success;
  }


  [[gnu::unused]]  ErrCode RecoPhoton::projectRecoConstraintOld(const FitParams& fitparams, Projection& p) const
  {

    ErrCode status ;
    // calculate the total momentum and energy:
    int momindex  = momIndex() ;
    int posindex  = mother()->posIndex();
    Eigen::Matrix<double, 1, 3> mom = Eigen::Matrix<double, 1, 3>::Zero(1, 3);
    Eigen::Matrix<double, 1, 3> dxVec = Eigen::Matrix<double, 1, 3>::Zero(1, 3);

    for (unsigned int i = 0; i < 3; ++i) {
      mom(i) = fitparams.getStateVector()(momindex + i);
      dxVec(i)      = m_params(i) - fitparams.getStateVector()(posindex + i);
    }

    double mom2 = mom * mom.transpose();
    double energy = sqrt(mom2) ;

    Eigen::Matrix<double, 3, 4> P = Eigen::Matrix<double, 3, 4>::Zero(3, 4);
    P(0, 0) = mom(1) + mom(2);
    P(0, 1) = -1.* mom(0);
    P(0, 2) = -1.* mom(0);
    P(1, 0) = -1.* mom(2);
    P(1, 1) = -1.* mom(2);
    P(1, 2) = mom(0) + mom(1);
    P(2, 3) = 1;
    // now get the residual. start in four dimensions
    Eigen::Matrix<double, 4, 1> residual4 = Eigen::Matrix<double, 4, 1>::Zero(4, 1);
    residual4.segment(0, 3) = dxVec.segment(0, 3);
    residual4(3) = m_params(3) - energy;
    // project out the 3D par

    Eigen::Matrix<double, 3, 1> r = P * residual4; // //JFK: maybe use .segment(0,3) 2017-10-12
    Eigen::Matrix<double, 3, 3> V = Eigen::Matrix<double, 3, 3>::Zero(3, 3);
    V = P * m_covariance.selfadjointView<Eigen::Lower>() * P.transpose();

    // calculate the parameter projection matrix
    // first the 'position' part
    // //JFK: we need H because of the dimensionality 2017-10-12
    Eigen::Matrix<double, 3, 7> H = Eigen::Matrix<double, 3, 7>::Zero(3, 7); // = -1. * P;
    // now the 'momentum' part
    H.block<3, 3>(0, 0) = -1.0 * P.block<3, 3>(0, 0);

    H(0, 3) = -1 * dxVec(1) - dxVec(2);
    H(0, 4) = dxVec(0);
    H(0, 5) = dxVec(0);
    H(1, 3) = dxVec(2);
    H(1, 4) = dxVec(2);
    H(1, 5) = -1 * dxVec(0) - dxVec(1);
    //H.block<1, 3>(2, 2) = mom.segment(0, 3) / energy;
    for (int col = 0; col < 3; ++col) {
      H(2, 3 + col) = -1 * mom(col) / energy ;
    }
    // done. copy everything back into the 'projection' object

//    std::cout << "m_params(3) " << m_params(3) << " energy " << energy  << std::endl;
//    std::cout << "P\n" << P  << std::endl;
//    std::cout << "residual4\n" << residual4  << std::endl;
//    std::cout << "r\n" << r  << std::endl;

    p.getResiduals().segment(0, 3) = r.segment(0, 3); // //JFK: this was maxsize == dimm before. Why? 2017-10-12
    //JFK: Eigen cant use blocks with variable size. for vectors it works 2017-09-26
    // fill the error matrix
    p.getV() = V.selfadjointView<Eigen::Lower>();
    // fill the projection
    //for (int row = 0; row < dimm; ++row) {
    for (int row = 0; row < 3; ++row) {
      for (int col = 0; col < 3; ++col) {
        p.getH()(row, posindex + col) =  H(row, col);
      }
      for (int col = 0; col < 3; ++col) {
        p.getH()(row, momindex + col) =  H(row, col + 3);
      }
    }

    return ErrCode::success; //FT: temp fix
  }




  ErrCode RecoPhoton::projectRecoConstraint(const FitParams& fitparams, Projection& p) const
  {
    ErrCode status ;
    const int momindex  = momIndex() ;
    const int posindex  = mother()->posIndex();

    const Eigen::Matrix<double, 1, 3> x_vec = fitparams.getStateVector().segment(posindex, 3);
    const Eigen::Matrix<double, 1, 3> p_vec = fitparams.getStateVector().segment(momindex, 3);

    const double mom = p_vec.norm();

    //Eigen::Matrix<double, 3, 4> P = Eigen::Matrix<double, 3, 4>::Zero(3, 4);
    //const double Ec2 = m_params(3) * m_params(3);

    const Eigen::Matrix<double, 1, 3> vertexToCluster = x_vec - m_params.segment(0, 3);
    const double delta = vertexToCluster.norm();

    //const double EcDelta = m_params(3) * delta;
    const double theta = delta / m_params(3);

    Eigen::Matrix<double, 4, 1> residual4 = Eigen::Matrix<double, 4, 1>::Zero(4, 1);
    for (unsigned int row = 0 ; row < 3; row++) {
      residual4(row) = m_params(row) - x_vec(row) - theta * p_vec(row);
    }
    residual4(3) = m_params(3) - mom;

    //P(0, 0) = 1;
    //P(0, 3) =     p_vec(0) * EcDelta / Ec2;

    //P(1, 1) = 1;
    //P(1, 3) =     p_vec(1) * EcDelta / Ec2;

    //P(2, 2) = 1;
    //P(2, 3) =     p_vec(2) * EcDelta / Ec2;

    //p.getV() = P * m_covariance.selfadjointView<Eigen::Lower>() * P.transpose();
    p.getV() = m_covariance.selfadjointView<Eigen::Lower>();
    //p.getResiduals().segment(0, 3) = residual4.segment(0, 3);
    p.getResiduals().segment(0, 4) = residual4.segment(0, 4);

    if (dim() > 3) {
      p.getH()(3, momindex)     =  p_vec(0) / mom;
      p.getH()(3, momindex + 1) =  p_vec(1) / mom;
      p.getH()(3, momindex + 2) =  p_vec(2) / mom;
    }

    for (unsigned int row = 0; row < 3; row++) {
      p.getH()(row, posindex + row) = 1;
      p.getH()(row, momindex + row) = theta;
    }

    return ErrCode::success;
  }

}

