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

//Handles the conversion of parameters between (x[3],p[3]) phase space and (helix[5], flight time) track representation

#include <iostream>
#include <TMath.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/Matrix.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <analysis/modules/TreeFitter/HelixUtils.h>

//#define OLDHELIX

using std::cout;
using std::endl;

namespace TreeFitter {

  extern int vtxverbose ;

  void HelixUtils::vertexFromHelix(const Belle2::Helix& helix,
                                   double L, double Bz,
                                   TVector3& position,
                                   TVector3& momentum, int& charge)
  {
    position = helix.getPositionAtArcLength2D(L);
    momentum = helix.getMomentumAtArcLength2D(L, Bz);
    charge = helix.getChargeSign();
  }

  //JFK: JUMP 2017-10-11
  void HelixUtils::helixFromVertex(Eigen::Matrix<double, 1, 6>& positionAndMomentum,
                                   int charge, double Bz,
                                   Belle2::Helix& helix,
                                   double& L,
                                   Eigen::Matrix<double, 5, 6>& jacobian)
  {

#ifdef OLDHELIX
    std::cout << "THIS SHOULD NEVER HAPPEN "  << std::endl;
#else

    TVector3 position(positionAndMomentum(0),
                      positionAndMomentum(1),
                      positionAndMomentum(2));
    TVector3 momentum(positionAndMomentum(3),
                      positionAndMomentum(4),
                      positionAndMomentum(5));

    helix = Belle2::Helix(position, momentum, charge, Bz);
    L = helix.getArcLength2DAtXY(positionAndMomentum(0),
                                 positionAndMomentum(1));

    const double alpha =  helix.getAlpha(Bz);

    //Copied from Belle2::UncertainHelix
    // COMPLETELY WRONG SINCE IT ASSUMES IT'S IN THE.operator() PERIGEE,
    // ONLY A PLACEHOLDER FOR NOW
    // 1. Rotate to a system where phi0 = 0
    Eigen::Matrix<double, 6, 6> jacobianRot = Eigen::Matrix<double, 6, 6>::Zero(6, 6);

    const double px = positionAndMomentum(3);
    const double py = positionAndMomentum(4);
    const double pt = hypot(px, py);
    const double cosPhi0 = px / pt;
    const double sinPhi0 = py / pt;

    // Passive rotation matrix by phi0:
    jacobianRot(iX, iX) = cosPhi0;
    jacobianRot(iX, iY) = sinPhi0;
    jacobianRot(iY, iX) = -sinPhi0;
    jacobianRot(iY, iY) = cosPhi0;
    jacobianRot(iZ, iZ) = 1.0;

    jacobianRot(iPx, iPx) = cosPhi0;
    jacobianRot(iPx, iPy) = sinPhi0;
    jacobianRot(iPy, iPx) = -sinPhi0;
    jacobianRot(iPy, iPy) = cosPhi0;
    jacobianRot(iPz, iPz) = 1.0;

    // 2. Translate to perigee parameters on the position
    const double pz = positionAndMomentum(5);
    const double invPt = 1 / pt;
    const double invPtSquared = invPt * invPt;
    Eigen::Matrix<double, 5, 6> jacobianToHelixParameters = Eigen::Matrix<double, 5, 6>::Zero(5, 6);
    jacobianToHelixParameters(iD0, iY) = -1;
    jacobianToHelixParameters(iPhi0, iX) = charge * invPt / alpha;
    jacobianToHelixParameters(iPhi0, iPy) = invPt;
    jacobianToHelixParameters(iOmega, iPx) = -charge * invPtSquared / alpha;
    jacobianToHelixParameters(iTanLambda, iPx) = - pz * invPtSquared;
    jacobianToHelixParameters(iTanLambda, iPz) = invPt;
    jacobianToHelixParameters(iZ0, iX) = - pz * invPt;
    jacobianToHelixParameters(iZ0, iZ) = 1;
    //
    jacobian = jacobianToHelixParameters * jacobianRot;

#endif
  }

  void HelixUtils::helixFromVertex(const TVector3& position,
                                   const TVector3& momentum,
                                   int charge, double Bz,
                                   Belle2::Helix& helix,
                                   double& L,
                                   CLHEP::HepMatrix& jacobian)
  {

#ifdef OLDHELIX

    // first copy
    double x  = position.X() ;
    double y  = position.Y() ;
    double z  = position.Z() ;
    double px = momentum.X() ;
    double py = momentum.Y() ;
    double pz = momentum.Z() ;
    helix = Belle2::Helix(position, momentum, charge, Bz);
    L = helix.getArcLength2DAtXY(position.X(), position.Y());
    const double alpha  =  helix.getAlpha(Bz);
    const double a = 1 / alpha;
    //    const double a  = Bz * Belle2::Unit::T * Belle2::Const::speedOfLight;

    // omega
    double aq    = a * charge ;
    double pt2   = px * px + py * py ;
    double pt    = sqrt(pt2) ;
    double omega = aq / pt ;
    cout << "calculated omega = " << omega << endl;
    cout << "helix omega = " << helix.getOmega() << endl;
    //    double omega = helix.getOmega();

    // now tandip
    double tandip = pz / pt ;
    cout << "calculated tanlambda = " << tandip << endl;
    cout << "helix tanlambda = " << helix.getTanLambda() << endl;
    //double tandip = helix.getTanLambda();

    // now phi0
    double phi     = atan2(py, px);
    double px0 = px + aq * y ;
    double py0 = py - aq * x ;
    double phi0    = atan2(py0, px0) ;
    cout << "calculated phi0 = " << phi0 << endl;
    cout << "helix phi0 = " << helix.getPhi0() << endl;
    //    double phi0    = helix.getPhi0();

    // now d0
    double pt02 = px0 * px0 + py0 * py0 ;
    double pt0 = sqrt(pt02) ;
    double d0 = - (pt0 - pt) / aq ;//FT changed sign temporarily
    cout << "calculated d0 = " << d0 << endl;
    cout << "helix d0 = " << helix.getD0() << endl;
    //    double d0 = helix.getD0();

    // now z0
    double deltaphi = phi - phi0 ;
    if (deltaphi >  TMath::Pi()) deltaphi -= TMath::TwoPi() ;
    else if (deltaphi < -TMath::Pi()) deltaphi += TMath::TwoPi() ;
    double l  = deltaphi / omega ;
    double z0 = z - tandip * l ;
    cout << "calculated z0 = " << z0 << endl;
    cout << "helix z0 = " << helix.getZ0() << endl;
    //    double z0 = helix.getZ0();

    // now L
    double p2 = pt2 + pz * pz ;
    double p  = sqrt(p2) ;
    cout << "calculated arclength = " << l* p / pt << endl;
    cout << "helix arclenght = " << L << endl;
    //    L = l * p / pt ;

    // all derivatives.
    double dptdpx    = px / pt ;
    double dptdpy    = py / pt ;
    double domegadpt = -omega / pt ;
    double dpt0dpx   = px0 / pt0 ;
    double dpt0dpy   = py0 / pt0 ;
    double dpt0dx    = -aq * dpt0dpy ;
    double dpt0dy    = aq * dpt0dpx ;
    double dpdpx     = px / p ;
    double dpdpy     = py / p ;

    double domegadpx = domegadpt * dptdpx ;
    double domegadpy = domegadpt * dptdpy ;

    double dphidpx = -py / pt2 ;
    double dphidpy = px / pt2 ;

    double dphi0dpx = -py0 / pt02 ;
    double dphi0dpy = px0 / pt02 ;
    double dphi0dx  = -aq * dphi0dpy ;
    double dphi0dy  = aq * dphi0dpx ;

    double dd0dpx = - (dpt0dpx - dptdpx) / aq ; //FT also changed sign to all derivatives
    double dd0dpy = - (dpt0dpy - dptdpy) / aq ;
    double dd0dx  = - dpt0dx / aq ;
    double dd0dy  = - dpt0dy / aq ;

    double dldx  = -dphi0dx / omega ;
    double dldy  = -dphi0dy / omega ;
    double dldpx = (dphidpx - dphi0dpx - l * domegadpx) / omega ;
    double dldpy = (dphidpy - dphi0dpy - l * domegadpy) / omega ;

    double dLdx  = dldx * L / l ;
    double dLdy  = dldy * L / l ;
    double dLdpx = L * (dldpx / l + dpdpx / p - dptdpx / pt) ;
    double dLdpy = L * (dldpy / l + dpdpy / p - dptdpy / pt) ;
    //double dLdpx = L*(dldpx/l - px*pz*pz/(p2*pt2)) ;
    //double dLdpy = L*(dldpy/l - py*pz*pz/(p2*pt2)) ;
    double dLdpz = l * pz / (pt * p) ;

    double dtandipdpx = -dptdpx * tandip / pt ;
    double dtandipdpy = -dptdpy * tandip / pt ;
    double dtandipdpz = 1 / pt ;

    double dz0dx  = -tandip * dldx ;
    double dz0dy  = -tandip * dldy ;
    double dz0dz  = 1;
    double dz0dpx = -tandip * dldpx - l * dtandipdpx ;
    double dz0dpy = -tandip * dldpy - l * dtandipdpy ;
    double dz0dpz = -l * dtandipdpz ;

    //now copy everything back
    //    helix = Belle2::Helix(d0,phi0,omega,z0,tandip);
    //   helix = Belle2::Helix(position, momentum,charge,Bz);
    // the row is helixpar, the column the vertexpar
    /*
    cout << "D0: " << d0 << " (BaBar) :" << helix.getD0() << " (Belle)" << endl;
    cout << "Phi0: " << phi0 << " (BaBar) :" << helix.getPhi0() << " (Belle)" << endl;
    cout << "Omega: " << omega << " (BaBar) :" << helix.getOmega() << " (Belle)" << endl;
    cout << "Z0: " << z0 << " (BaBar) :" << helix.getZ0() << " (Belle)" << endl;
    cout << "TanLambda: " << tandip << " (BaBar) :" << helix.getTanLambda() << " (Belle)" << endl;
    */
    if (jacobian.num_col() != 6 || jacobian.num_row() != 5)
      jacobian = CLHEP::HepMatrix(5, 6) ;

    if (jacobian.num_col() == 6 && jacobian.num_row() == 5) {
      for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 6; ++col)
          jacobian[row][col] = 0 ;

      jacobian[iOmega][iX] = 0 ;
      jacobian[iOmega][iY] = 0 ;
      jacobian[iOmega][iZ] = 0 ;
      jacobian[iOmega][iPx] = domegadpx ;
      jacobian[iOmega][iPy] = domegadpy ;
      jacobian[iOmega][iPz] = 0 ;

      jacobian[iPhi0][iX]  = dphi0dx ;
      jacobian[iPhi0][iY]  = dphi0dy ;
      jacobian[iPhi0][iZ]  = 0 ;
      jacobian[iPhi0][iPx] = dphi0dpx ;
      jacobian[iPhi0][iPy] = dphi0dpy ;
      jacobian[iPhi0][iPz] = 0 ;

      jacobian[iD0][iX]  = dd0dx ;
      jacobian[iD0][iY]  = dd0dy ;
      jacobian[iD0][iZ]  = 0 ;
      jacobian[iD0][iPx] = dd0dpx ;
      jacobian[iD0][iPy] = dd0dpy ;
      jacobian[iD0][iPz] = 0 ;

      jacobian[iTanLambda][iX] = 0 ;
      jacobian[iTanLambda][iY] = 0 ;
      jacobian[iTanLambda][iZ] = 0 ;
      jacobian[iTanLambda][iPx] = dtandipdpx ;
      jacobian[iTanLambda][iPy] = dtandipdpy ;
      jacobian[iTanLambda][iPz] = dtandipdpz ;

      jacobian[iZ0][iX]  = dz0dx ;
      jacobian[iZ0][iY]  = dz0dy ;
      jacobian[iZ0][iZ]  = dz0dz ;
      jacobian[iZ0][iPx] = dz0dpx ;
      jacobian[iZ0][iPy] = dz0dpy ;
      jacobian[iZ0][iPz] = dz0dpz ;
      /*
      jacobian[iArcLength2D][iX]  = dLdx ;
      jacobian[iArcLength2D][iY]  = dLdy ;
      jacobian[iArcLength2D][iZ]  = 0 ;
      jacobian[iArcLength2D][iPx] = dLdpx ;
      jacobian[iArcLength2D][iPy] = dLdpy ;
      jacobian[iArcLength2D][iPz] = dLdpz ;
      */
    }
#else
    helix = Belle2::Helix(position, momentum, charge, Bz);
    L = helix.getArcLength2DAtXY(position.X(), position.Y());
    const double alpha =  helix.getAlpha(Bz);

    //Copied from Belle2::UncertainHelix
    // COMPLETELY WRONG SINCE IT ASSUMES IT'S IN THE PERIGEE,
    // ONLY A PLACEHOLDER FOR NOW
    // 1. Rotate to a system where phi0 = 0
    TMatrixD jacobianRot(6, 6);
    jacobianRot.Zero();

    const double px = momentum.X();
    const double py = momentum.Y();
    const double pt = hypot(px, py);
    const double cosPhi0 = px / pt;
    const double sinPhi0 = py / pt;

    // Passive rotation matrix by phi0:
    jacobianRot(iX, iX) = cosPhi0;
    jacobianRot(iX, iY) = sinPhi0;
    jacobianRot(iY, iX) = -sinPhi0;
    jacobianRot(iY, iY) = cosPhi0;
    jacobianRot(iZ, iZ) = 1.0;

    jacobianRot(iPx, iPx) = cosPhi0;
    jacobianRot(iPx, iPy) = sinPhi0;
    jacobianRot(iPy, iPx) = -sinPhi0;
    jacobianRot(iPy, iPy) = cosPhi0;
    jacobianRot(iPz, iPz) = 1.0;

    // 2. Translate to perigee parameters on the position
    const double pz = momentum.Z();
    const double invPt = 1 / pt;
    const double invPtSquared = invPt * invPt;
    TMatrixD jacobianToHelixParameters(5, 6);
    jacobianToHelixParameters.Zero();
    jacobianToHelixParameters(iD0, iY) = -1;
    jacobianToHelixParameters(iPhi0, iX) = charge * invPt / alpha;
    jacobianToHelixParameters(iPhi0, iPy) = invPt;
    jacobianToHelixParameters(iOmega, iPx) = -charge * invPtSquared / alpha;
    jacobianToHelixParameters(iTanLambda, iPx) = - pz * invPtSquared;
    jacobianToHelixParameters(iTanLambda, iPz) = invPt;
    jacobianToHelixParameters(iZ0, iX) = - pz * invPt;
    jacobianToHelixParameters(iZ0, iZ) = 1;

    //then jacobian = jacobianToHelix*jacobianRot
    TMatrixD jacobianTmp(5, 6);
    jacobianTmp = jacobianToHelixParameters * jacobianRot;

    //    cout << "The Jacobian is:" << endl;
    //    jacobianTmp.Print();

    // trivial cast from TMatrix to HepMatrix
    // the row is helixpar, the column the vertexpar
    if (jacobian.num_col() != 6 || jacobian.num_row() != 5)
      jacobian = CLHEP::HepMatrix(5, 6) ;
    if (jacobian.num_col() == 6 && jacobian.num_row() == 5) {
      for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 6; ++col)
          jacobian[row][col] = jacobianTmp(row, col);
    }
#endif
  }

  std::string HelixUtils::helixParName(int i)
  {
    std::string rc ;
    switch (i) {
      case 1     : rc = "d0    : " ; break ;
      case 2     : rc = "phi0  : " ; break ;
      case 3     : rc = "omega : " ; break ;
      case 4     : rc = "z0    : " ; break ;
      case 5     : rc = "tandip: " ; break ;
      case 6     : rc = "L     : " ; break ;
    }
    return rc ;
  }

  std::string HelixUtils::vertexParName(int i)
  {
    std::string rc ;
    switch (i) {
      case 1  : rc = "x    : " ; break ;
      case 2  : rc = "y    : " ; break ;
      case 3  : rc = "z    : " ; break ;
      case 4  : rc = "px   : " ; break ;
      case 5  : rc = "py   : " ; break ;
      case 6  : rc = "pz   : " ; break ;
    }
    return rc ;
  }

  void HelixUtils::printHelixPar(const CLHEP::HepVector& helixpar)
  {
    for (int i = 0; i < 6; ++i)
      cout << helixParName(i + 1).c_str() << helixpar[i] << endl ;
  }

  void HelixUtils::printVertexPar(const TVector3& position, const TVector3& momentum, int charge)
  {
    for (int i = 0; i < 3; ++i)
      cout << vertexParName(i + 1).c_str() << position[i] << endl ;
    for (int i = 0; i < 3; ++i)
      cout << vertexParName(i + 4).c_str() << momentum[i] << endl ;
    cout << "charge:    " << charge << endl ;
  }

  //JFK: FIXME JUMP 2017-10-11
  //Calculate Jacobian numerically. Precision is questionable, but you don't have to get the derivative calculation right... good for cross checks
  void HelixUtils::getHelixAndJacobianFromVertexNumerical(Eigen::Matrix<double, 1, 6>& positionAndMom,
                                                          int charge, double Bz,
                                                          Belle2::Helix& helix,
                                                          Eigen::Matrix<double, 5, 6>& jacobian)
  {

    TVector3 position(positionAndMom(0),
                      positionAndMom(1),
                      positionAndMom(2));
    TVector3 momentum(positionAndMom(3),
                      positionAndMom(4),
                      positionAndMom(5));

    helix = Belle2::Helix(position, momentum, charge, Bz);

    // numeric calculation of the jacobian
    Belle2::Helix helixPlusDelta;

    double delta = 1e-5 ;// this is quite a random choice.

    TVector3 postmp;
    TVector3 momtmp;

    for (int jin = 0; jin < 6; ++jin) {
      for (int i = 0; i < 3; ++i) {
        postmp[i] = positionAndMom(i);
        momtmp[i] = positionAndMom(i + 3);
      }
      if (jin < 3) {
        postmp[jin] += delta;
      } else {
        momtmp[jin - 3] += delta;
      }
      helixPlusDelta = Belle2::Helix(postmp, momtmp, charge, Bz);
      jacobian(iD0, jin)        = (helixPlusDelta.getD0()        - helix.getD0())        / delta ;
      jacobian(iPhi0, jin)      = (helixPlusDelta.getPhi0()      - helix.getPhi0())      / delta ;
      jacobian(iOmega, jin)     = (helixPlusDelta.getOmega()     - helix.getOmega())     / delta ;
      jacobian(iZ0, jin)        = (helixPlusDelta.getZ0()        - helix.getZ0())        / delta ;
      jacobian(iTanLambda, jin) = (helixPlusDelta.getTanLambda() - helix.getTanLambda()) / delta ;

      //      jacobian[iArcLength2D][jin] = (LPlusDelta - L) / delta ;
    }
    //    cout << "Numerical Jacobian: " << endl << jacobian << endl;
  }




  //Calculate Jacobian numerically. Precision is questionable, but you don't have to get the derivative calculation right... good for cross checks
  void HelixUtils::helixFromVertexNumerical(const TVector3& position,
                                            const TVector3& momentum,
                                            int charge, double Bz,
                                            Belle2::Helix& helix,
                                            double& L,
                                            CLHEP::HepMatrix& jacobian)
  {
    // first call with dummy jacobian
    CLHEP::HepMatrix dummy ;
    HelixUtils::helixFromVertex(position, momentum, charge, Bz, helix, L, dummy) ;

    // numeric calculation of the jacobian
    TVector3 postmp;
    TVector3 momtmp;
    Belle2::Helix helixtmp;
    double Ltmp;
    CLHEP::HepMatrix jacobiantmp(5, 6) ;

    //double delta = 0.001*abs(vertexpar[jin]) ;
    //if(delta < 1e-8) delta = 1e-8 ;
    double delta = 1e-5 ;// this is quite a random choice.
    for (int jin = 0; jin < 6; ++jin) {
      for (int i = 0; i < 3; ++i) {
        postmp[i] = position[i];
        momtmp[i] = momentum[i];
      }
      if (jin < 3) postmp[jin] += delta;
      else momtmp[jin - 3] += delta;
      //
      HelixUtils::helixFromVertex(postmp, momtmp, charge, Bz, helixtmp, Ltmp, jacobiantmp) ;
      jacobian[iD0][jin] = (helixtmp.getD0() - helix.getD0()) / delta ;
      jacobian[iPhi0][jin] = (helixtmp.getPhi0() - helix.getPhi0()) / delta ;
      jacobian[iOmega][jin] = (helixtmp.getOmega() - helix.getOmega()) / delta ;
      jacobian[iZ0][jin] = (helixtmp.getZ0() - helix.getZ0()) / delta ;
      jacobian[iTanLambda][jin] = (helixtmp.getTanLambda() - helix.getTanLambda()) / delta ;
      //      jacobian[iArcLength2D][jin] = (Ltmp - L) / delta ;
    }
    //    cout << "Numerical Jacobian: " << endl << jacobian << endl;
  }

  inline double sqr(double x) { return x * x ; }

  double HelixUtils::phidomain(const double phi)
  {
    double rc = phi ;
    if (phi < -TMath::Pi())  rc += TMath::TwoPi();
    else if (phi >  TMath::Pi())  rc -= TMath::TwoPi();
    return rc ;
  }

  //POCA between two tracks
  double HelixUtils::helixPoca(const Belle2::Helix& helix1,
                               const Belle2::Helix& helix2,
                               double& flt1, double& flt2,
                               TVector3& vertex, bool parallel)
  {

    double d0_1     = helix1.getD0();
    double phi0_1   = helix1.getPhi0();
    double omega_1  = helix1.getOmega();
    double z0_1     = helix1.getZ0();
    double tandip_1 = helix1.getTanLambda();
    double cosdip_1 = cos(atan(tandip_1))  ; // can do that faster

    double d0_2     = helix2.getD0();
    double phi0_2   = helix2.getPhi0();
    double omega_2  = helix2.getOmega();
    double z0_2     = helix2.getZ0();
    double tandip_2 = helix2.getTanLambda();
    double cosdip_2 = cos(atan(tandip_2))  ; // can do that faster

    double r_1 = 1 / omega_1 ;
    double r_2 = 1 / omega_2 ;

    double x0_1 = - (r_1 + d0_1) * sin(phi0_1) ;
    double y0_1 = (r_1 + d0_1) * cos(phi0_1) ;

    double x0_2 = - (r_2 + d0_2) * sin(phi0_2) ;
    double y0_2 = (r_2 + d0_2) * cos(phi0_2) ;

    double deltax = x0_2 - x0_1 ;
    double deltay = y0_2 - y0_1 ;

    double phi1[2] ;
    double phi2[2] ;
    int nsolutions = 1;

    // the phi of the 'intersection'.
    const double pi = TMath::Pi();
    double phi    = - atan2(deltax, deltay) ;
    double phinot = phi > 0 ?  phi - pi : phi + pi ;
    phi1[0] = r_1 < 0 ? phi : phinot ;
    phi2[0] = r_2 > 0 ? phi : phinot ;

    double R1 = fabs(r_1) ;
    double R2 = fabs(r_2) ;
    double Rmin = R1 < R2 ? R1 : R2 ;
    double Rmax = R1 > R2 ? R1 : R2 ;
    double dX = sqrt(deltax * deltax + deltay * deltay) ;

    if (!parallel && dX + Rmin > Rmax && dX < R1 + R2) {
      // there are two solutions
      nsolutions = 2 ;
      double ddphi1 = acos((dX * dX - R2 * R2 + R1 * R1) / (2.*dX * R1)) ;
      phi1[1] = phidomain(phi1[0] + ddphi1) ;
      phi1[0] = phidomain(phi1[0] - ddphi1)  ;

      double ddphi2 = acos((dX * dX - R1 * R1 + R2 * R2) / (2.*dX * R2)) ;
      phi2[1] = phidomain(phi2[0] - ddphi2) ;
      phi2[0] = phidomain(phi2[0] + ddphi2) ;

    } else if (dX < Rmax) {
      if (R1 > R2) phi2[0] = r_2 < 0 ? phi : phinot ;
      else          phi1[0] = r_1 < 0 ? phi : phinot ;
    }

//     cout << "nsolutions: " << nsolutions << endl ;
//     cout << "xydist,R,r1,r2: " << dX << " " << Rmin << " " << Rmax << " "
//   <<  r_1 << " " << r_2 << endl ;
//     cout << "pars: "
//   << x0_1 << "," << y0_1 << "," << r_1 << ","
//     << x0_2 << "," << y0_2 << "," << r_2 << endl ;

    // find the best solution for z by running multiples of 2_pi
    double z1(0), z2(0) ;
    bool first(true) ;
    int ibest = 0;
    const int ncirc(2) ;
    for (int i = 0; i < nsolutions; ++i) {
      double dphi1 = phidomain(phi1[i] - phi0_1) ;
      double dphi2 = phidomain(phi2[i] - phi0_2) ;
      for (int n1 = 1 - ncirc; n1 <= 1 + ncirc ; ++n1) {
        double l1 = (dphi1 + n1 * TMath::TwoPi()) / omega_1 ;
        double tmpz1 = (z0_1 + l1 * tandip_1) ;
        if (n1 == 0 || fabs(tmpz1) < 100) {
          for (int n2 = 1 - ncirc ; n2 <= 1 + ncirc; ++n2) {
            double l2 = (dphi2 + n2 * TMath::TwoPi()) / omega_2 ;
            double tmpz2 = (z0_2 + l2 * tandip_2) ;
            if (n2 == 0 || fabs(tmpz2) < 100) {
              //      cout << "n1,n2: " << i << " " << n1 << " " << n2 << " "
              //     << l1/cosdip_1<< " " << l2/cosdip_2 << endl ;
              if (first || fabs(tmpz1 - tmpz2) < fabs(z1 - z2)) {
                ibest = i ;
                first = false ;
                z1 = tmpz1 ;
                z2 = tmpz2 ;
                flt1 = l1 / cosdip_1 ;
                flt2 = l2 / cosdip_2 ;
              }
            }
          }
        }
      }
    }

    double x1 =  r_1 * sin(phi1[ibest]) + x0_1 ;
    double y1 = -r_1 * cos(phi1[ibest]) + y0_1 ;

    double x2 =  r_2 * sin(phi2[ibest]) + x0_2 ;
    double y2 = -r_2 * cos(phi2[ibest]) + y0_2 ;

    //     cout << "bestz1,bestz2: " << bestz1 << " " << bestz2 << endl ;
    //     cout << "bestx1,bestx2: " << x1 << " " << x2 << endl ;
    //     cout << "besty1,besty2: " << y1 << " " << y2 << endl ;

    vertex.SetX(0.5 * (x1 + x2));
    vertex.SetY(0.5 * (y1 + y2));
    vertex.SetZ(0.5 * (z1 + z2));
    //    vertex = CLHEP::HepPoint( 0.5*(x1+x2), 0.5*(y1+y2), 0.5*(z1+z2) ) ;
    return sqrt(sqr(x2 - x1) + sqr(y2 - y1) + sqr(z2 - z1)) ;
  }

  //POCA between a track and a point
  double HelixUtils::helixPoca(const Belle2::Helix& helix,
                               const TVector3& point,
                               double& flt)
  {
    double d0     = helix.getD0();
    double phi0   = helix.getPhi0();
    double omega  = helix.getOmega();
    double z0     = helix.getZ0();
    double tandip = helix.getTanLambda();
    double cosdip = cos(atan(tandip))  ; // can do that faster

    double r = 1 / omega ;

    double x0 = - (r + d0) * sin(phi0) ;
    double y0 = (r + d0) * cos(phi0) ;

    double deltax = x0 - point.X() ;
    double deltay = y0 - point.Y() ;

    const double pi = TMath::Pi();
    double phi    = - atan2(deltax, deltay) ;
    if (r < 0) phi = phi > 0 ?  phi - pi : phi + pi ;

    // find the best solution for z by running multiples of 2_pi
    double x =  r * sin(phi) + x0 ;
    double y = -r * cos(phi) + y0 ;
    double z(0) ;
    bool first(true) ;
    const int ncirc(2) ;
    double dphi = phidomain(phi - phi0) ;
    for (int n = 1 - ncirc; n <= 1 + ncirc ; ++n) {
      double l = (dphi + n * TMath::TwoPi()) / omega ;
      double tmpz = (z0 + l * tandip) ;
      if (first || fabs(tmpz - point.z()) < fabs(z - point.z())) {
        first = false ;
        z = tmpz ;
        flt = l / cosdip ;
      }
    }
    return sqrt(sqr(x - point.x()) + sqr(y - point.y()) + sqr(z - point.z())) ;
  }

  void HelixUtils::helixTest()
  {
    const double pi = 3.1415927 ;
    CLHEP::HepVector helixpar(6) ;

    helixpar[HelixUtils::iD0]          = 2 ;
    helixpar[HelixUtils::iPhi0]        = +pi - 0.1;
    helixpar[HelixUtils::iOmega]       = 0.05 ;
    helixpar[HelixUtils::iZ0]          = 0.5 ;
    helixpar[HelixUtils::iTanLambda]   = -7 ;
    helixpar[HelixUtils::iArcLength2D] = 10 ;

    cout << "This goes in: " << endl ;
    HelixUtils::printHelixPar(helixpar) ;

    TVector3 position;
    TVector3 momentum;
    int charge ;
    double Bz = 1.5;
    HelixUtils::vertexFromHelix(Belle2::Helix(helixpar[HelixUtils::iD0],
                                              helixpar[HelixUtils::iPhi0],
                                              helixpar[HelixUtils::iOmega],
                                              helixpar[HelixUtils::iZ0],
                                              helixpar[HelixUtils::iTanLambda]),
                                helixpar[HelixUtils::iArcLength2D], Bz,
                                position, momentum, charge);

    cout << "This convertes to: " << endl ;
    HelixUtils::printVertexPar(position, momentum, charge) ;

    Belle2::Helix helixback;
    double Lback;
    CLHEP::HepMatrix jacobian(5, 6) ;
    HelixUtils::helixFromVertex(position, momentum, charge, Bz, helixback, Lback, jacobian);

    cout << "We get back: " << endl ;
    CLHEP::HepVector helixparback(6);
    helixparback[HelixUtils::iD0]          = helixback.getD0();
    helixparback[HelixUtils::iPhi0]        = helixback.getPhi0();
    helixparback[HelixUtils::iOmega]       = helixback.getOmega();
    helixparback[HelixUtils::iZ0]          = helixback.getZ0();
    helixparback[HelixUtils::iTanLambda]   = helixback.getTanLambda();
    helixparback[HelixUtils::iArcLength2D] = Lback;
    HelixUtils::printHelixPar(helixparback) ;
    cout << "Analytic Jacobian: " << endl << jacobian << endl;
    // numeric check of the jacobian
    CLHEP::HepMatrix jacobiannum(6, 6) ;
    HelixUtils::helixFromVertexNumerical(position, momentum, charge, Bz, helixback, Lback, jacobiannum) ;
    //
    for (int iex = 0; iex < 5; ++iex) {
      for (int jin = 0; jin < 6; ++jin) {
        double anaderiv = jacobian[iex][jin] ;
        double numderiv = jacobiannum[iex][jin] ;
        cout << "d" << HelixUtils::helixParName(iex + 1)
             << "/d" << HelixUtils::vertexParName(jin + 1)
             << " =  " << numderiv << " (num), " << anaderiv << " (anal)" << endl ;
      }
    }
  }


}
