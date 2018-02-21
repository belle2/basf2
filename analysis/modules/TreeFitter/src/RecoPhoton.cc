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

#include <analysis/modules/TreeFitter/RecoPhoton.h>
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/HelixUtils.h>
#include <analysis/modules/TreeFitter/ErrCode.h>

#include <framework/gearbox/Const.h>

namespace TreeFitter {
  extern int vtxverbose ;

  RecoPhoton::RecoPhoton(Belle2::Particle* particle, const ParticleBase* mother)
    : RecoParticle(particle, mother),
      m_init(false),
      m_useEnergy(useEnergy(*particle)),
      m_m(4),
      m_matrixV(4),
      m_params(),//use params for dim if klongs come into the game
      m_covariance()
  {
    updateParams() ;
  }

  RecoPhoton::~RecoPhoton()
  {
  }

  ErrCode RecoPhoton::initParticleWithMother(FitParams* fitparams)
  {
    // calculate the direction
    int posindexmother = mother()->posIndex();

    Eigen::Matrix<double, 1, 3> posVec = Eigen::Matrix<double, 1, 3>::Zero(1, 3);
    for (unsigned int i = 0; i < 3; ++i) {
      posVec(i) = m_params(i) - fitparams->getStateVector()(posindexmother + i);
    }
    double deltaX2 = posVec * posVec.transpose();
    // get the energy
    //JFK: FIXME THIS WILL MAKE A SEGFAULT 2017-09-26
    double energy = m_useEnergy ? m_params(3) : particle()->getEnergy();
    int momindex = momIndex();
    for (unsigned int i = 0; i < 3; ++i) {
      fitparams->getStateVector()(momindex + i) = energy / sqrt(deltaX2) * posVec(i);
    }
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

  ErrCode RecoPhoton::initPar2(FitParams* fitparams)
  {
    // calculate the direction
    int posindexmother = mother()->posIndex() ;
    CLHEP::HepVector deltaX(3);
    double deltaX2(0);
    for (int row = 1; row <= 3; ++row) {
      double dx = m_m(row) - fitparams->par(posindexmother + row);
      deltaX(row) = dx;
      deltaX2 += dx * dx;
    }
    // get the energy
    double energy = m_useEnergy ? m_m(4) : particle()->getEnergy();
    // assign the momentum
    int momindex = momIndex();
    for (int row = 1; row <= 3; ++row)
      fitparams->par(momindex + row) = energy * deltaX(row) / sqrt(deltaX2);
    return ErrCode();
  }

  ErrCode RecoPhoton::initCov(FitParams* fitparams) const
  {
    int momindex = momIndex() ;
    double varEnergy =  m_useEnergy ? m_matrixV.fast(4, 4) : 1;
    const double factor = 1000;
    for (int row = 1; row <= 3; ++row)
      fitparams->cov()(momindex + row, momindex + row) = factor * varEnergy;

    return ErrCode();
  }

  ErrCode RecoPhoton::initCovariance(FitParams* fitparams) const
  {
    int momindex = momIndex() ;
    double varEnergy =  m_useEnergy ? m_covariance(3, 3) : 1;
    const double factor = 1000;
//    fitparams->getCovariance().block<3, 3>(momindex, momindex) =
//      Eigen::Matrix<double, 3, 3>::Constant(3, 3, factor * varEnergy);

    for (int row = 0; row < 3; ++row) {
      fitparams->getCovariance()(momindex + row, momindex + row) = factor * varEnergy;
    }
    return ErrCode();
  }

  ErrCode RecoPhoton::updCache()
  {
    const Belle2::ECLCluster* recoCalo = particle()->getECLCluster();
    TVector3 centroid = recoCalo->getClusterPosition();
    double energy = recoCalo->getEnergy();
    m_init = true;

    // This returns the covariance matrix assuming the photons comes from the nominal IP
    Belle2::ClusterUtils C;
    TMatrixDSym cov_pE = C.GetCovarianceMatrix4x4FromCluster(
                           recoCalo);//FT: error on xyz is extracted from error on p (sort of backwards but ok)

    for (int row = 1; row <= 4; ++row)
      for (int col = row; col <= 4; ++col)
        m_matrixV(row, col) = cov_pE[row - 1][col - 1];
    //
    m_m(1) = centroid.X() ;
    m_m(2) = centroid.Y() ;
    m_m(3) = centroid.Z() ;
    if (m_useEnergy) m_m(4) = energy ;
    if (vtxverbose >= 3) {
      std::ostringstream stream;
      stream << "Neutral particle: " << particle()->getPDGCode() << "\n"
             << "Measurement is: (" << m_m(1) << "," << m_m(2) << "," << m_m(3);
      if (m_useEnergy) stream << "," << m_m(4);
      stream << ")\n"
             << "energy is: " << energy
             << "cov matrix is: " << m_matrixV;
      B2DEBUG(19, stream.str());
    }
    return ErrCode() ;
  }
  ErrCode RecoPhoton::updateParams()
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


  ErrCode RecoPhoton::projectRecoConstraint(const FitParams& fitparams, Projection& p) const
  {
    // residual of photon:
    // r(1-3) = motherpos + mu * photon momentum - cluster position
    // r(4)   = |momentum| - cluster energy
    // mu is calculated from the 'chi2-closest approach' (see below)
    ErrCode status ;

    // calculate the total momentum and energy:
    int momindex  = momIndex() ;
    CLHEP::HepVector mom = fitparams.par().sub(momindex + 1, momindex + 3) ;
    double mom2 = mom.normsq() ;
    double mass = pdgMass() ;
    double energy = sqrt(mass * mass + mom2) ;

    // calculate dX = Xc - Xmother
    int posindex  = mother()->posIndex() ;
    CLHEP::HepVector dX(3);
    for (int row = 1; row <= 3; ++row)
      dX(row) = m_m(row) - fitparams.par(posindex + row) ;

    // the constraints we will use are (dX = Xc - Xmother)
    //  I) r(1) = py * dX - px * dY + pz * dX - px * dZ
    // II) r(2) = px * dZ - pz * dX + py * dZ - pz * dY
    //III) r(3) = Ec - energy
    //
    // We will need two projection matrices:
    // a) the matrix that projects on the measurement parameters (=P)
    // b) the matrix that projects on the fit parameters (=H)
    //
    // create the matrix that projects the measurement in the constraint equations
    // this would all be easier if we had tensors. no such luck.
    CLHEP::HepMatrix P(3, 4, 0) ;
    P(1, 1) = mom(2) + mom(3) ; P(1, 2) = -mom(1) ; P(1, 3) = -mom(1) ;
    P(2, 1) = -mom(3) ;         P(2, 2) = -mom(3) ; P(2, 3) = mom(1) + mom(2) ;
    P(3, 4) = 1 ;
    // now get the residual. start in four dimensions
    CLHEP::HepVector residual4(4);
    for (int row = 1; row <= 3; ++row) residual4(row) = dX(row) ;
    residual4(4) = m_m(4) - energy ;
    // project out the 3D par
    CLHEP::HepVector       r = P * residual4 ;
    CLHEP::HepSymMatrix    V = m_matrixV.similarity(P) ;
    // calculate the parameter projection matrix
    // first the 'position' part
    CLHEP::HepMatrix H(3, 7, 0) ;
    for (int irow = 1; irow <= 3; ++irow)
      for (int icol = 1; icol <= 3; ++icol)
        H(irow, icol) = - P(irow, icol) ;

    // now the 'momentum' part
    H(1, 4) = -dX(2) - dX(3) ;    H(1, 5) = dX(1) ;   H(1, 6) = dX(1) ;
    H(2, 4) =  dX(3) ;          H(2, 5) = dX(3) ;   H(2, 6) = -dX(1) - dX(2) ;
    for (int col = 1; col <= 3; ++col)
      H(3, 3 + col) = -mom(col) / energy ;
    // done. copy everything back into the 'projection' object
    int dimm = dimM(); // if we don't use the energy, this is where it will drop out

    for (int row = 1; row < dimm; ++row)
      p.r(row) = r(row) ;

    // fill the error matrix
    for (int row = 1; row <= dimm; ++row)
      for (int col = 1; col <= row; ++col)
        p.Vfast(row, col) = V.fast(row, col) ;

    // fill the projection
    for (int row = 1; row <= dimm; ++row) {
      for (int col = 1; col <= 3; ++col)
        p.H(row, posindex + col) = H(row, col) ;
      for (int col = 1; col <= 3; ++col)
        p.H(row, momindex + col) = H(row, col + 3) ;
    }
    //    return status ;
    return ErrCode::success; //FT: temp fix
  }
  ErrCode RecoPhoton::projectRecoConstraintCopy(const FitParams& fitparams, Projection& p) const
  {
    // residual of photon:
    // r(1-3) = motherpos + mu * photon momentum - cluster position
    // r(4)   = |momentum| - cluster energy
    // mu is calculated from the 'chi2-closest approach' (see below)
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
    double mass = pdgMass() ;
    double energy = sqrt(mass * mass + mom2) ;
    // calculate dX = Xc - Xmother
    // the constraints we will use are (dX = Xc - Xmother)
    //  I) r(1) = py * dX - px * dY + pz * dX - px * dZ
    // II) r(2) = px * d - pz * dX + py * dZ - pz * dY
    //III) r(3) = Ec - energy
    //
    // We will need two projection matrices:
    // a) the matrix that projects on the measurement parameters (=P)
    // b) the matrix that projects on the fit parameters (=H)
    //
    // create the matrix that projects the measurement in the constraint equations
    // this would all be easier if we had tensors. no such luck.
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
    //JFK: instead of H = - P, we change all signs in the following 2017-09-26
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
    //int dimm = dimM(); // if we don't use the energy, this is where it will drop out
    p.getResiduals().segment(0, 3) = r.segment(0, 3); // //JFK: this was maxsize == dimm before. Why? 2017-10-12
    //JFK: Eigen cant use blocks with variable size. for vectors it works 2017-09-26
    // fill the error matrix
    //p.getV().block<3, 3>(0, 0) = V.selfadjointView<Eigen::Lower>();
    p.getV() = V.selfadjointView<Eigen::Lower>();
    //for (int row = 0; row < dimm; ++row) {
    //  for (int col = 0; col <= row; ++col) {
    //    p.getV()(row, col) = V(row, col);
    //  }
    //}
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
    //B2DEBUG(19, "reco photon r\n" << r  );
//    B2DEBUG(19, "reco photon P\n" << P  );
//    B2DEBUG(19, "reco photon H\n" << H  );

    return ErrCode::success; //FT: temp fix
  }



}

