/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <TMath.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>

namespace TreeFitter {

  void HelixUtils::vertexFromHelix(const Belle2::Helix& helix,
                                   double L, double Bz,
                                   Belle2::B2Vector3D& position,
                                   Belle2::B2Vector3D& momentum, int& charge)
  {
    position = helix.getPositionAtArcLength2D(L);
    momentum = helix.getMomentumAtArcLength2D(L, Bz);
    charge = helix.getChargeSign();
  }

  void HelixUtils::helixFromVertex(const Eigen::Matrix<double, 1, 6>& positionAndMomentum,
                                   int charge, double Bz,
                                   Belle2::Helix& helix,
                                   double& L,
                                   Eigen::Matrix<double, 5, 6>& jacobian)
  {


    Belle2::B2Vector3D position(positionAndMomentum(0),
                                positionAndMomentum(1),
                                positionAndMomentum(2));
    Belle2::B2Vector3D momentum(positionAndMomentum(3),
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

  void HelixUtils::printVertexPar(const Belle2::B2Vector3D& position, const Belle2::B2Vector3D& momentum, int charge)
  {
    for (int i = 0; i < 3; ++i)
      B2INFO(vertexParName(i + 1).c_str() << position[i]);
    for (int i = 0; i < 3; ++i)
      B2INFO(vertexParName(i + 4).c_str() << momentum[i]);
    B2INFO("charge:    " << charge);

  }

  void HelixUtils::getHelixAndJacobianFromVertexNumerical(const Eigen::Matrix<double, 1, 6>& positionAndMom,
                                                          int charge, double Bz,
                                                          Belle2::Helix& helix,
                                                          // cppcheck-suppress constParameter ; jacobian is updated below
                                                          Eigen::Matrix<double, 5, 6>& jacobian)
  {

    Belle2::B2Vector3D position(positionAndMom(0),
                                positionAndMom(1),
                                positionAndMom(2));

    Belle2::B2Vector3D momentum(positionAndMom(3),
                                positionAndMom(4),
                                positionAndMom(5));

    helix = Belle2::Helix(position, momentum, charge, Bz);

    // numeric calculation of the jacobian
    Belle2::Helix helixPlusDelta;

    double delta = 1e-5;// this is arbitrary, only needs to be small

    Belle2::B2Vector3D postmp;
    Belle2::B2Vector3D momtmp;

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
    const Eigen::Matrix<double, 1, 6>& positionAndMom,
    int charge, double Bz,
    const Belle2::Helix& helix,
    // cppcheck-suppress constParameter ; jacobian is assigned new matrix elements below
    Eigen::Matrix<double, 5, 6>& jacobian,
    double delta
  )
  {
    // numeric calculation of the jacobian
    Belle2::Helix helixPlusDelta;

    Belle2::B2Vector3D postmp;
    Belle2::B2Vector3D momtmp;

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
                               Belle2::B2Vector3D& vertex, bool parallel)
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
                               const Belle2::B2Vector3D& point,
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

  // cppcheck-suppress constParameter ; jacobian is clearly updated in the function
  void HelixUtils::getJacobianToCartesianFrameworkHelix(Eigen::Matrix<double, 5, 6>& jacobian,
                                                        const double x,
                                                        const double y,
                                                        const double z __attribute__((unused)),
                                                        const double px,
                                                        const double py,
                                                        const double pz,
                                                        const double bfield,
                                                        const double charge
                                                       )

  {
    const double alpha = 1.0 / (bfield * Belle2::Const::speedOfLight) * 1E4;
    const double aq = charge / alpha;

    const double pt = std::hypot(px, py);
    const double pt2 = pt * pt;
    const double pt3 = pt2 * pt;
    const double aq2 = aq * aq;

    const double x2 = x * x;
    const double y2 = y * y;
    const double r  = x2 + y2;

    const double px2 = px * px;
    const double py2 = py * py;

    const double px0 = px - aq * y;
    const double py0 = py + aq * x;

    const double pt02 = px0 * px0 + py0 * py0;
    const double pt0 = std::sqrt(pt02);
    double sqrt13 = pt0 / pt;

    // D d0 / Dx_i
    jacobian(0, 0) = py0 / pt0;
    jacobian(0, 1) = -px0 / pt0;
    jacobian(0, 2) = 0;
    jacobian(0, 3) = (-(y * (aq2 * r + 2 * aq * py * x + 2 * py2 * (1 + sqrt13))) - px * (2 * py * x * (1 + sqrt13) + aq * (y2 *
                      (-1 + sqrt13) + x2 * (1 + sqrt13)))) /
                     (pt2 * pt0 * (1 + sqrt13) * (1 + sqrt13));

    jacobian(0, 4) = (2 * px2 * x * (1 + sqrt13) + 2 * px * y * (py - aq * x + py * sqrt13) + aq * (aq * r * x - py * (x2 *
                      (-1 + sqrt13) + y2 * (1 + sqrt13)))) /
                     (pt2 * pt0 * (1 + sqrt13) * (1 + sqrt13));
    jacobian(0, 5) = 0;

    // D phi0 / Dx_i0;
    jacobian(1, 0) = aq * px0 / pt02;
    jacobian(1, 1) = aq * py0 / pt02;
    jacobian(1, 2) = 0;
    jacobian(1, 3) = -py0 / pt02;
    jacobian(1, 4) = px0 / pt02;
    jacobian(1, 5) = 0;

    // D omega / Dx_i
    jacobian(2, 0) = 0;
    jacobian(2, 1) = 0;
    jacobian(2, 2) = 0;
    jacobian(2, 3) = - aq * px / pt3;
    jacobian(2, 4) = - aq * py / pt3;
    jacobian(2, 5) = 0;

    // D z0 / Dx_i
    jacobian(3, 0) = -pz * px0 / pt02;
    jacobian(3, 1) = -pz * py0 / pt02;
    jacobian(3, 2) = 1;
    jacobian(3, 3) = (pz * (px2 * x - py * (aq * r + py * x) + 2 * px * py * y)) / (pt2 * pt02);
    jacobian(3, 4) = (pz * (px * (aq * r + 2 * py * x) - px2 * y + py2 * y)) / (pt2 * pt02);
    jacobian(3, 5) = std::atan2(-(aq * (px * x + py * y)), (px2 + py * py0 - aq * px * y)) / aq; //pt on num. and denom cancels.

    // D tan lambda / Dx_i
    jacobian(4, 0) = 0;
    jacobian(4, 1) = 0;
    jacobian(4, 2) = 0;
    jacobian(4, 3) = -pz * px / pt3;
    jacobian(4, 4) = -pz * py / pt3;
    jacobian(4, 5) = 1. / pt;
  }

}
