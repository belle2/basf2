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


#include <iostream>
#include <TMath.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>

//#define OLDHELIX

namespace TreeFitter {
  /** there is no std::sign() */
  template <typename T> int inline sgn(T val)
  {
    return (T(0) < val) - (val < T(0));
  }



  void HelixUtils::vertexFromHelix(const Belle2::Helix& helix,
                                   double L, double Bz,
                                   TVector3& position,
                                   TVector3& momentum, int& charge)
  {
    position = helix.getPositionAtArcLength2D(L);
    momentum = helix.getMomentumAtArcLength2D(L, Bz);
    charge = helix.getChargeSign();
  }

  void HelixUtils::helixFromVertex(Eigen::Matrix<double, 1, 6>& positionAndMomentum,
                                   int charge, double Bz,
                                   Belle2::Helix& helix,
                                   double& L,
                                   Eigen::Matrix<double, 5, 6>& jacobian)
  {

#ifdef OLDHELIX
    B2ERROR("THIS SHOULD NEVER HAPPEN ");
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

  void HelixUtils::printVertexPar(const TVector3& position, const TVector3& momentum, int charge)
  {
    for (int i = 0; i < 3; ++i)
      B2INFO(vertexParName(i + 1).c_str() << position[i]);
    for (int i = 0; i < 3; ++i)
      B2INFO(vertexParName(i + 4).c_str() << momentum[i]);
    B2INFO("charge:    " << charge);

  }

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

    double delta = 1e-5;// this is arbitrary, only needs to be small

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
  }

  void HelixUtils::getJacobianFromVertexNumerical(
    Eigen::Matrix<double, 1, 6>& positionAndMom,
    int charge, double Bz,
    Belle2::Helix& helix,
    Eigen::Matrix<double, 5, 6>& jacobian,
    double delta
  )
  {
    // numeric calculation of the jacobian
    Belle2::Helix helixPlusDelta;

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
    }

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

    vertex.SetX(0.5 * (x1 + x2));
    vertex.SetY(0.5 * (y1 + y2));
    vertex.SetZ(0.5 * (z1 + z2));

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

  //Make this into a unit test:

//  void HelixUtils::helixTest()
//  {
//    const double pi = 3.1415927 ;
//    CLHEP::HepVector helixpar(6) ;
//
//    helixpar[HelixUtils::iD0]          = 2 ;
//    helixpar[HelixUtils::iPhi0]        = +pi - 0.1;
//    helixpar[HelixUtils::iOmega]       = 0.05 ;
//    helixpar[HelixUtils::iZ0]          = 0.5 ;
//    helixpar[HelixUtils::iTanLambda]   = -7 ;
//    helixpar[HelixUtils::iArcLength2D] = 10 ;
//
//    B2INFO("This goes in: " );
//    HelixUtils::printHelixPar(helixpar) ;
//
//    TVector3 position;
//    TVector3 momentum;
//    int charge ;
//    double Bz = 1.5;
//    HelixUtils::vertexFromHelix(Belle2::Helix(helixpar[HelixUtils::iD0],
//                                              helixpar[HelixUtils::iPhi0],
//                                              helixpar[HelixUtils::iOmega],
//                                              helixpar[HelixUtils::iZ0],
//                                              helixpar[HelixUtils::iTanLambda]),
//                                helixpar[HelixUtils::iArcLength2D], Bz,
//                                position, momentum, charge);
//
//    B2INFO("This convertes to: " );
//    HelixUtils::printVertexPar(position, momentum, charge) ;
//
//    Belle2::Helix helixback;
//    double Lback;
//    CLHEP::HepMatrix jacobian(5, 6) ;
//    HelixUtils::helixFromVertex(position, momentum, charge, Bz, helixback, Lback, jacobian);
//
//    B2INFO("We get back: " );
//    CLHEP::HepVector helixparback(6);
//    helixparback[HelixUtils::iD0]          = helixback.getD0();
//    helixparback[HelixUtils::iPhi0]        = helixback.getPhi0();
//    helixparback[HelixUtils::iOmega]       = helixback.getOmega();
//    helixparback[HelixUtils::iZ0]          = helixback.getZ0();
//    helixparback[HelixUtils::iTanLambda]   = helixback.getTanLambda();
//    helixparback[HelixUtils::iArcLength2D] = Lback;
//    HelixUtils::printHelixPar(helixparback) ;
//    B2DEBUG(19, "Analytic Jacobian: " << endl << jacobian);
//    // numeric check of the jacobian
//    CLHEP::HepMatrix jacobiannum(6, 6) ;
//    HelixUtils::helixFromVertexNumerical(position, momentum, charge, Bz, helixback, Lback, jacobiannum) ;
//    //
//    for (int iex = 0; iex < 5; ++iex) {
//      for (int jin = 0; jin < 6; ++jin) {
//        double anaderiv = jacobian[iex][jin] ;
//        double numderiv = jacobiannum[iex][jin] ;
//        B2DEBUG(19, "d" << HelixUtils::helixParName(iex + 1));
//             << "/d" << HelixUtils::vertexParName(jin + 1)
//             << " =  " << numderiv << " (num), " << anaderiv << " (anal)" );
//      }
//    }
//  }
  void HelixUtils::getJacobianToCartesianFrameworkHelix(Eigen::Matrix<double, 5, 6>& jacobian,
                                                        const double x,
                                                        const double y,
                                                        const double z,
                                                        const double px,
                                                        const double py,
                                                        const double pz,
                                                        const double bfield,
                                                        const double charge
                                                       )

  {
    const double alpha = 1.0 / (bfield * Belle2::Const::speedOfLight) * 1E4;
    const double aq = charge / alpha;


    const double phi = std::atan2(py, px);
    const double pt = std::hypot(px, py);
    const double pt2 = pt * pt;
    const double pt3 = pt2 * pt;
    const double pt5 = pt2 * pt3;
    const double pt7 = pt3 * pt5;
    const double tanLambda = pz / pt;
    const double aq2 = aq * aq;

    const double cosPhi = std::cos(phi);
    const double sinPhi = std::sin(phi);

    const double para = -x * cosPhi - y * sinPhi; //delta parallel
    const double para2 = para * para;
    const double ortho = -y * cosPhi + x * sinPhi; //delta orthogonal

    // pt * ( 1+ qa*ortho/pt ) term in Dz0/Dx
    const double a = pt + aq * ortho;
    const double a2 = a * a;


    const double x2 = x * x;
    const double y2 = y * y;
    const double x3 = x2 * x;
    const double y3 = y2 * y;
    const double px2 = px * px;
    const double px3 = px2 * px;
    const double px4 = px3 * px;
    const double py2 = py * py;
    const double py3 = py2 * py;
    const double py4 = py3 * py;

    const double r = x2 + y2;
    const double po = std::sqrt(1 + py2 / px2);

    // first term for 00
    const double t00_lead = (po * pt * px * py + aq * px2 * x + aq * py2 * x);
    const double sqrt001 = std::sqrt((pt * (py2 + aq2 * r) + 2 * aq * po * px * py * x + px2 * (pt - 2 * aq * po * y)) /
                                     pt3); // correct! as t03 is correct
    const double t001 = (-2 * px4 - py2 * (2 * py2 + aq2 * r) - px2 * (4 * py2 + aq2 * r) - 2 * aq * po * pt * px * py * x + 2 * aq * po
                         * pt * px2 * y - 2 * px2 * sqrt001 - 4 * px2 * py2 * sqrt001 - 2 * py4 * sqrt001);
    const double t00num = t00_lead * t001;

    const double denum00 = pt7 * sqrt001 * (1 + sqrt001) * (1 + sqrt001);

    const double t00 = -t00num / denum00;

    const double abspx = std::abs(px);
    const double sqrtabspx = std::sqrt(abspx);
    const double sqrtabspx3 = sqrtabspx * sqrtabspx * sqrtabspx;
    const double a_03 = (2 * aq * px * (py * x - px * y) + abspx * (pt2 + aq2 * r));
    const double sqrt03denum = std::sqrt(abspx) + std::sqrt(a_03 / pt2);
    const double denum03 = pt2 * pt2 * std::sqrt(a_03) + sqrt03denum * sqrt03denum;

    const double num03 = (-(pt2 * px * (2 * px * py * x + (2 * py2 + aq2 * r) * y) * sqrtabspx)
                          - aq * pt2 * (2 * py * x * y + px * (x - y) * (x + y)) * sqrtabspx3
                          - 2 * px * py * (px * x + py * y) * std::sqrt(pt2 * (2 * aq * px * (py * x - px * y) + (pt2 + aq2 * r) * abspx))
                          - aq * px * r * abspx * std::sqrt(pt2 * (2 * aq * px * (py * x - px * y) + (pt2 + aq2 * r) * abspx)));

    const double num04 = (pt2 * px * (2 * px2 * x + aq2 * r * x + 2 * px * py * y) * sqrtabspx
                          - aq * pt2 * (2 * px * x * y + py * (-(x * x) + y * y)) * sqrtabspx3
                          + 2 * px2 * (px * x + py * y) * std::sqrt(pt2 * (2 * aq * px * (py * x - px * y) + (pt2 + aq2 * r) * abspx))
                          - aq * py * r * abspx * std::sqrt(pt2 * (2 * aq * px * (py * x - px * y) + (pt2 + aq2 * r) * abspx)));

    const double numerator10 = aq * (aq * py2 * y + px2 * (-pt * po + aq * y));
    const double denominator10 = px4 + 2 * aq * px * py * pt * po * x + py4 + py2 * aq2 * r + px2 * (2 * py2 + aq *
                                 (-2 * pt * po * y + aq * r));
    const double numerator11 = aq * (px * py * pt * po + aq * px2 * x + aq * py2 * x);

    const double numerator33 = (pz * (-px4 * x + py4 * x - 2 * px3 * py * y + px * (-2 * py3 * y + aq * py * pt * po *
                                      r))); //correct to 1e6
    const double denominator33 = px * pt * po * (px4 + 2 * aq * px * py * pt * po * x + py2 * (py2 + aq2 * r) + px2 * (2 * py2 + aq *
                                                 (-2 * pt * po * y + r * aq)));
    const double numerator34 = (pz * (2 * px3 * py * x + 2 * px * py3 * x - px4 * y + py4 * y + aq * px2 * pt * po * r));

    const double signPhiByP = 0 > px ? 1 : -1;
    const double px0 = px + signPhiByP * aq * y;
    const double px02 = px0 * px0;
    const double py0 = py - signPhiByP * aq * x;
    const double py02 = py0 * py0;

    double phi13 = 0;
    double phi14 = 0;
    if (px < 0) { phi13 = (-2 * py) / pt2 + (py - aq * x) / ((py - aq * x) * (py - aq * x) + (px + aq * y) * (px + aq * y)) ;}
    else { phi13 = (-2 * py) / pt2 + (py + aq * x) / ((py + aq * x) * (py + aq * x) + (px - aq * y) * (px - aq * y)) ;}

    if (px < 0) { phi14 = (2 * px) / pt2 - (px + aq * y) / ((py - aq * x) * (py - aq * x) + (px + aq * y) * (px + aq * y)) ;}
    else { phi14 = (2 * px) / pt2 + (-px + aq * y) / ((py + aq * x) * (py + aq * x) + (px - aq * y) * (px - aq * y)) ;}


    double sqrt13 = std::sqrt(((py + aq * x) * (py + aq * x) + (px - aq * y) * (px - aq * y)) / pt2);

    // D d0 / Dx_i
    jacobian(0, 0) = (px * py + aq * x * std::abs(px)) / std::sqrt(px * (px * (pt2 + aq2 * r) + 2 * aq * (py * x - px * y) * std::abs(
                       px)));
    jacobian(0, 1) = (aq * y - std::abs(px)) / sqrt(pt2 + aq2 * r + 2 * aq * (py * x - px * y) * sgn(px));
    jacobian(0, 2) = 0; //done
    jacobian(0, 3) = (-(y * (aq2 * r + 2 * aq * py * x + 2 * py2 * (1 + sqrt13))) - px * (2 * py * x * (1 + sqrt13) + aq * (y2 *
                      (-1 + sqrt13) + x2 * (1 + sqrt13)))) / (pt2 * std::sqrt((py + aq * x) * (py + aq * x) + (px - aq * y) * (px - aq * y)) *
                                                              (1 + sqrt13) * (1 + sqrt13)); // done no mismatch
    jacobian(0, 4) = (2 * px2 * x * (1 + sqrt13) + 2 * px * y * (py - aq * x + py * sqrt13) + aq * (aq * r * x - py * (x2 *
                      (-1 + sqrt13) + y2 * (1 + sqrt13)))) / (pt2 * std::sqrt((py + aq * x) * (py + aq * x) + (px - aq * y) * (px - aq * y)) *
                                                              (1 + sqrt13) * (1 + sqrt13));
    jacobian(0, 5) = 0; //done

    // D phi0 / Dx_i0;
    jacobian(1, 0) = -numerator10 / denominator10; //done
    jacobian(1, 1) = numerator11 / denominator10; //done
    jacobian(1, 2) = 0; //done


    jacobian(1, 3) = -(2 * py / pt2) + (py + aq * x) / ((py + aq * x) * (py + aq * x) + (px - aq * y) * (px - aq * y));
    jacobian(1, 4) = (2 * px) / pt2 + (-px + aq * y) / ((py + aq * x) * (py + aq * x) + (px - aq * y) * (px - aq * y)); //
    jacobian(1, 5) = 0;//done

    // D omega / Dx_i
    jacobian(2, 0) = 0; //done
    jacobian(2, 1) = 0; //done
    jacobian(2, 2) = 0; //done
    jacobian(2, 3) = - aq * px / pt3; //done
    jacobian(2, 4) = - aq * py / pt3; //done
    jacobian(2, 5) = 0; //done

    // D z0 / Dx_i
    jacobian(3, 0) = pz * (-1 * aq2 * sinPhi * para / a2 - aq * cosPhi / a) / (aq * (1 + aq2 * para2 / a2)); //done
    jacobian(3, 1) = pz * (aq2 * cosPhi * para / a2 - aq * sinPhi / a) / (aq * (1 + aq2 * para2 / a2));   //done
    jacobian(3, 2) = 1; //done
    jacobian(3, 3) = - (numerator33 / denominator33); //done
    jacobian(3, 4) = numerator34 / denominator33;    //done
    jacobian(3, 5) = 1. / aq * std::atan(aq * (px * x + py * y) / (-px * pt * po - aq * py * x + aq * px *
                                         y)); //done TODO is atan save here?

    // D tan lambda / Dx_i
    jacobian(4, 0) = 0;//done
    jacobian(4, 1) = 0;//done
    jacobian(4, 2) = 0;//done
    jacobian(4, 3) = - pz * px / pt3; //done
    jacobian(4, 4) = - pz * py / pt3; //done
    jacobian(4, 5) = 1. / pt; //done
  }

}
