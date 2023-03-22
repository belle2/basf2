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

#include <algorithm>
#include <initializer_list>
#include <vector>

namespace TreeFitter {

  void HelixUtils::vertexFromHelix(const Belle2::Helix& helix,
                                   double L, double Bz,
                                   ROOT::Math::XYZVector& position,
                                   ROOT::Math::XYZVector& momentum, int& charge)
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

    helix = Belle2::Helix(ROOT::Math::XYZVector(positionAndMomentum(0), positionAndMomentum(1), positionAndMomentum(2)),
                          ROOT::Math::XYZVector(positionAndMomentum(3), positionAndMomentum(4), positionAndMomentum(5)),
                          charge, Bz);

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
                                                          Eigen::Matrix<double, 5, 6>& jacobian)
  {

    helix = Belle2::Helix(ROOT::Math::XYZVector(positionAndMom(0), positionAndMom(1), positionAndMom(2)),
                          ROOT::Math::XYZVector(positionAndMom(3), positionAndMom(4), positionAndMom(5)),
                          charge, Bz);

    // numeric calculation of the jacobian
    Belle2::Helix helixPlusDelta;

    double delta = 1e-5;// this is arbitrary, only needs to be small

    ROOT::Math::XYZVector postmp;
    ROOT::Math::XYZVector momtmp;

    for (int jin = 0; jin < 6; ++jin) {
      postmp.SetCoordinates(positionAndMom(0), positionAndMom(1), positionAndMom(2));
      momtmp.SetCoordinates(positionAndMom(3), positionAndMom(4), positionAndMom(5));
      if (jin == 0) postmp.SetX(postmp.X() + delta);
      if (jin == 1) postmp.SetY(postmp.Y() + delta);
      if (jin == 2) postmp.SetZ(postmp.Z() + delta);
      if (jin == 3) momtmp.SetX(momtmp.X() + delta);
      if (jin == 4) momtmp.SetY(momtmp.Y() + delta);
      if (jin == 5) momtmp.SetZ(momtmp.Z() + delta);

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
    Eigen::Matrix<double, 5, 6>& jacobian,
    double delta
  )
  {
    // numeric calculation of the jacobian
    Belle2::Helix helixPlusDelta;

    ROOT::Math::XYZVector postmp;
    ROOT::Math::XYZVector momtmp;

    for (int jin = 0; jin < 6; ++jin) {
      postmp.SetCoordinates(positionAndMom(0), positionAndMom(1), positionAndMom(2));
      momtmp.SetCoordinates(positionAndMom(3), positionAndMom(4), positionAndMom(5));
      if (jin == 0) postmp.SetX(postmp.X() + delta);
      if (jin == 1) postmp.SetY(postmp.Y() + delta);
      if (jin == 2) postmp.SetZ(postmp.Z() + delta);
      if (jin == 3) momtmp.SetX(momtmp.X() + delta);
      if (jin == 4) momtmp.SetY(momtmp.Y() + delta);
      if (jin == 5) momtmp.SetZ(momtmp.Z() + delta);

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

    const double d0_1     = helix1.getD0();
    const double phi0_1   = helix1.getPhi0();
    const double omega_1  = helix1.getOmega();

    const double d0_2     = helix2.getD0();
    const double phi0_2   = helix2.getPhi0();
    const double omega_2  = helix2.getOmega();

    // These radii have a sign, like omega (negative for negative charge)
    const double r_1 = 1 / omega_1 ;
    const double r_2 = 1 / omega_2 ;

    // 1) First look at the transverse plane, where the helix projection is a circle
    // Coordinates of the centers of the circles
    const double x0_1 = (r_1 + d0_1) * sin(phi0_1) ;
    const double y0_1 = -(r_1 + d0_1) * cos(phi0_1) ;

    const double x0_2 = (r_2 + d0_2) * sin(phi0_2) ;
    const double y0_2 = -(r_2 + d0_2) * cos(phi0_2) ;

    // Vector that goes from center1 to center2
    const double deltax = x0_2 - x0_1 ;
    const double deltay = y0_2 - y0_1 ;

    // Intersections of the circles, can be at most two
    double phi1[2] ;
    double phi2[2] ;
    int nsolutions = 1;

    // The phi of the delta vector.
    const double phi    = - atan2(deltax, deltay) ;
    const double phinot = phi > 0 ?  phi - TMath::Pi() : phi + TMath::Pi() ;
    phi1[0] = r_1 < 0 ? phi : phinot ;
    phi2[0] = r_2 > 0 ? phi : phinot ;

    // These radii do NOT have a sign instead
    const double R1 = fabs(r_1) ;
    const double R2 = fabs(r_2) ;
    const double Rmin = R1 < R2 ? R1 : R2 ;
    const double Rmax = R1 > R2 ? R1 : R2 ;
    const double dX = hypot(deltax, deltay) ;

    if (!parallel && dX + Rmin > Rmax && dX < R1 + R2) {
      // Circles intersect in two points
      nsolutions = 2 ;

      // This is just the law of cosines
      const double ddphi1 = acos((dX * dX - R2 * R2 + R1 * R1) / (2.*dX * R1)) ;
      phi1[1] = phidomain(phi1[0] + ddphi1) ;
      phi1[0] = phidomain(phi1[0] - ddphi1)  ;

      const double ddphi2 = acos((dX * dX - R1 * R1 + R2 * R2) / (2.*dX * R2)) ;
      phi2[1] = phidomain(phi2[0] - ddphi2) ;
      phi2[0] = phidomain(phi2[0] + ddphi2) ;

    } else if (dX < Rmax) {
      // Tangent or non-intersecting circles, one inside the other (only one POCA)
      if (R1 > R2) phi2[0] = r_2 < 0 ? phi : phinot ;
      else         phi1[0] = r_1 < 0 ? phi : phinot ;
    }
    // else: tangent or non-intersecting circles, outside of each other (only one POCA)
    // what we saved in phi1 and phi2 gives already the correct solution

    // Intersections of the circles (cartesian)
    double x1[2], y1[2], x2[2], y2[2];
    for (int i = 0; i < nsolutions; i++) {
      x1[i] =  r_1 * sin(phi1[i]) + x0_1 ;
      y1[i] = -r_1 * cos(phi1[i]) + y0_1 ;
      x2[i] =  r_2 * sin(phi2[i]) + x0_2 ;
      y2[i] = -r_2 * cos(phi2[i]) + y0_2 ;
    }

    // 2) Find the best solution for z by running multiples of 2pi from the xy intersection(s)
    double z1, z2;
    bool first = true;
    int ibest = 0;
    const int nturnsmax = 10; // Max number of turns we try backwards and forwards

    // Loop on all xy-plane solutions
    for (int i = 0; i < nsolutions; ++i) {
      const double l1 = helix1.getArcLength2DAtXY(x1[i], y1[i]);
      const double l2 = helix2.getArcLength2DAtXY(x2[i], y2[i]);

      // Loop on helix1 turns, save corresponding z positions
      std::vector<double> z1s;
      for (int n1 = 0; n1 <= nturnsmax; ++n1) {
        bool added = false;
        // Try forwards and backwards
        for (int sn1 : {n1, -n1}) {
          const double tmpz1 = helix1.getPositionAtArcLength2D(l1 + sn1 * TMath::TwoPi() / omega_1).Z();
          if (sn1 == 0 || (-82 <= tmpz1 && tmpz1 <= 158)) {
            // Only keep the 0th turn and those inside CDC volume
            z1s.push_back(tmpz1);
            added = true;
          }
          if (sn1 == 0)
            break; // Do not store 0th turn twice
        }
        // If we did not add any point we are already outside CDC volume both backwards and forwards
        if (!added)
          break;
      }

      // Loop on helix2 turns, find closest approach to one of helix1 points
      for (int n2 = 0; n2 <= nturnsmax; ++n2) {
        bool tried = false;
        // Try forwards and backwards
        for (int sn2 : {n2, -n2}) {
          const double tmpz2 = helix2.getPositionAtArcLength2D(l2 + sn2 * TMath::TwoPi() / omega_2).Z();
          if (sn2 == 0 || (-82 <= tmpz2 && tmpz2 <= 158)) {
            // Only keep the 0th turn and those inside CDC volume
            tried = true;
            // Find the tmpz1 closest to tmpz2
            const auto i1best = std::min_element(
            z1s.cbegin(), z1s.cend(), [&tmpz2](const double & z1a, const double & z1b) {
              return fabs(z1a - tmpz2) < fabs(z1b - tmpz2);
            });
            const double tmpz1 = *i1best;
            // Keep the solution where the z distance of closest approach is minimum
            if (first || fabs(tmpz1 - tmpz2) < fabs(z1 - z2)) {
              ibest = i;
              first = false;
              z1 = tmpz1;
              z2 = tmpz2;
              flt1 = l1;
              flt2 = l2;
            }
          }
          if (n2 == 0)
            break; // Do not try 0th turn twice
        }
        // If we did not try any point we are already outside CDC volume both backwards and forwards
        if (!tried)
          break;
      }
    }

    vertex.SetX(0.5 * (x1[ibest] + x2[ibest]));
    vertex.SetY(0.5 * (y1[ibest] + y2[ibest]));
    vertex.SetZ(0.5 * (z1 + z2));

    return hypot(x2[ibest] - x1[ibest], y2[ibest] - y1[ibest], z2 - z1);
  }

  //POCA between a track and a point
  double HelixUtils::helixPoca(const Belle2::Helix& helix,
                               const Belle2::B2Vector3D& point,
                               double& flt)
  {
    const double d0     = helix.getD0();
    const double phi0   = helix.getPhi0();
    const double omega  = helix.getOmega();
    const double z0     = helix.getZ0();
    const double tandip = helix.getTanLambda();
    const double cosdip = cos(atan(tandip))  ; // can do that faster

    const double r = 1 / omega ;

    const double x0 = - (r + d0) * sin(phi0) ;
    const double y0 = (r + d0) * cos(phi0) ;

    const double deltax = x0 - point.X() ;
    const double deltay = y0 - point.Y() ;

    const double pi = TMath::Pi();
    double phi = - atan2(deltax, deltay) ;
    if (r < 0) phi = phi > 0 ?  phi - pi : phi + pi ;

    // find the best solution for z by running multiples of 2_pi
    const double x =  r * sin(phi) + x0 ;
    const double y = -r * cos(phi) + y0 ;
    double z(0) ;
    bool first(true) ;
    const int ncirc(2) ;
    const double dphi = phidomain(phi - phi0) ;
    for (int n = 1 - ncirc; n <= 1 + ncirc ; ++n) {
      const double l = (dphi + n * TMath::TwoPi()) / omega ;
      const double tmpz = (z0 + l * tandip) ;
      if (first || fabs(tmpz - point.Z()) < fabs(z - point.Z())) {
        first = false ;
        z = tmpz ;
        flt = l / cosdip ;
      }
    }
    return sqrt(sqr(x - point.X()) + sqr(y - point.Y()) + sqr(z - point.Z())) ;
  }

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
