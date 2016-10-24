/**************************************************************************
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

#include <analysis/modules/TreeFitter/RecoPhoton.h>
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/HelixUtils.h>
#include <analysis/modules/TreeFitter/ErrCode.h>

#include <framework/gearbox/Const.h>

namespace TreeFitter {
  extern int vtxverbose ;

  //FT: this is needed once Klongs become involved
  bool RecoPhoton::useEnergy(Particle& particle)
  {
    bool rc = true ;
    int pdg = particle.getPDGCode();
    if (pdg &&
        Const::ParticleType(pdg) != Const::photon && //   pdg != 22 &&
        Const::ParticleType(pdg) != Const::pi0) { //   pdg != 111){
      rc = false ;
    }
    //    B2DEBUG(80, "RecoPhoton::useEnergy = " << rc << " , pdg = " << pdg);
    return rc ;
  }

  RecoPhoton::RecoPhoton(Particle* particle, const ParticleBase* mother)
    : RecoParticle(particle, mother), m_init(false), m_useEnergy(useEnergy(*particle)), m_m(4),
      m_matrixV(4) //FT: hardcoded, should use dimM()+1 as far as I can tell, dimensionality here needs a look over
      //    : RecoParticle(particle,mother),m_init(false),m_useEnergy(useEnergy(*particle)),m_m(dimM()),m_matrixV(dimM())
  {
    updCache() ;
  }

  RecoPhoton::~RecoPhoton() {}

  ErrCode RecoPhoton::initPar2(FitParams* fitparams)
  {
    // calculate the direction
    int posindexmother = mother()->posIndex() ;
    HepVector deltaX(3);
    double deltaX2(0);
    for (int row = 1; row <= 3; ++row) {
      double dx = m_m(row) - fitparams->par(posindexmother + row) ;
      deltaX(row) = dx ;
      deltaX2 += dx * dx ;
    }

    // get the energy
    double energy = m_useEnergy ? m_m(4) : particle()->getEnergy();

    // assign the momentum
    int momindex = momIndex();
    for (int row = 1; row <= 3; ++row)
      fitparams->par(momindex + row) = energy * deltaX(row) / sqrt(deltaX2)  ;
    return ErrCode();
  }

  ErrCode RecoPhoton::initCov(FitParams* fitparams) const
  {
    int momindex = momIndex() ;
    double varEnergy =  m_useEnergy ? m_matrixV.fast(4, 4) : 1 ;
    const double factor = 1000;
    for (int row = 1; row <= 3; ++row)
      fitparams->cov()(momindex + row, momindex + row) = factor * varEnergy ;
    return ErrCode();
  }

  ErrCode RecoPhoton::updCache()
  {
    const ECLCluster* recoCalo = particle()->getECLCluster();
    TVector3 centroid = recoCalo->getclusterPosition();
    double energy = recoCalo->getEnergy();
    m_init = true ;
    TMatrixDSym cov_pE = recoCalo->getError4x4();//FT: error on xyz is extracted from error on p (sort of backwards but ok)
    for (int row = 1; row <= 4; ++row)
      for (int col = row; col <= 4; ++col)
        m_matrixV(row, col) = cov_pE[row - 1][col - 1];
    //
    m_m(1) = centroid.X() ;
    m_m(2) = centroid.Y() ;
    m_m(3) = centroid.Z() ;
    if (m_useEnergy) m_m(4) = energy ;
    if (vtxverbose >= 3) {
      std::cout << "Neutral particle: " << particle()->getPDGCode() << std::endl
                << "Measurement is: (" << m_m(1) << "," << m_m(2) << "," << m_m(3);
      if (m_useEnergy) std::cout << "," << m_m(4);
      std::cout << ")" << std::endl
                << "energy is: " << energy << std::flush
                << "cov matrix is: " << m_matrixV << std::flush ;
    }
    return ErrCode() ;
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
    HepVector mom = fitparams.par().sub(momindex + 1, momindex + 3) ;
    double mom2 = mom.normsq() ;
    double mass = pdgMass() ;
    double energy = sqrt(mass * mass + mom2) ;

    // calculate dX = Xc - Xmother
    int posindex  = mother()->posIndex() ;
    HepVector dX(3);
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
    HepMatrix P(3, 4, 0) ;
    P(1, 1) = mom(2) + mom(3) ; P(1, 2) = -mom(1) ; P(1, 3) = -mom(1) ;
    P(2, 1) = -mom(3) ;         P(2, 2) = -mom(3) ; P(2, 3) = mom(1) + mom(2) ;
    P(3, 4) = 1 ;
    // now get the residual. start in four dimensions
    HepVector residual4(4);
    for (int row = 1; row <= 3; ++row) residual4(row) = dX(row) ;
    residual4(4) = m_m(4) - energy ;
    // project out the 3D par
    HepVector       r = P * residual4 ;
    HepSymMatrix    V = m_matrixV.similarity(P) ;
    // calculate the parameter projection matrix
    // first the 'position' part
    HepMatrix H(3, 7, 0) ;
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
}
