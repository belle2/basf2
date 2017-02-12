/**************************************************************************
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
#include "analysis/modules/TreeFitter/HelixUtils.h"

using std::cout;
using std::endl;

namespace TreeFitter {

  extern int vtxverbose ;

  void HelixUtils::vertexFromHelix(const HepVector& helixpar, double Bz,
                                   HepVector& vertexpar, int& charge)
  {
    double d0     = helixpar[ex_d0]     ;
    double phi0   = helixpar[ex_phi0]   ;
    double omega  = helixpar[ex_omega]  ;
    double z0     = helixpar[ex_z0]     ;
    double tandip = helixpar[ex_tanDip] ;
    double L      = helixpar[ex_flt]    ;

    double r = 1 / omega ;
    double l = L * cos(atan(tandip)) ;
    double phi = phi0 + omega * l ;
    double x  =  r * sin(phi) - (r + d0) * sin(phi0) ;
    double y  = -r * cos(phi) + (r + d0) * cos(phi0) ;
    double z  = z0 + l * tandip ;

    const double a  = Bz * Belle2::Unit::T * Belle2::Const::speedOfLight; //FT: a>0 for in Belle2 unlike BaBar

    charge = omega > 0 ? 1 : -1 ; //FT: charge and omega have the same sign in Belle2
    double pt = a * charge / omega ;
    double px = pt * cos(phi) ;
    double py = pt * sin(phi) ;
    double pz = pt * tandip ;

    if (vertexpar.num_row() != 6)  vertexpar = HepVector(6) ;
    vertexpar[in_x]  = x  ;
    vertexpar[in_y]  = y  ;
    vertexpar[in_z]  = z  ;
    vertexpar[in_px] = px ;
    vertexpar[in_py] = py ;
    vertexpar[in_pz] = pz ;
  }

  void HelixUtils::helixFromVertex(const HepVector& vertexpar, int charge,
                                   double Bz, HepVector& helixpar, HepMatrix& jacobian)
  {
    // first copy
    double x  = vertexpar[in_x] ;
    double y  = vertexpar[in_y] ;
    double z  = vertexpar[in_z] ;
    double px = vertexpar[in_px] ;
    double py = vertexpar[in_py] ;
    double pz = vertexpar[in_pz] ;

    const double a  = Bz * Belle2::Unit::T * Belle2::Const::speedOfLight; //FT: a>0 for in Belle2 unlike BaBar

    // omega
    double aq    = a * charge ;
    double pt2   = px * px + py * py ;
    double pt    = sqrt(pt2) ;
    double omega = aq / pt ;

    // now tandip
    double tandip = pz / pt ;

    // now phi0
    double phi     = atan2(py, px);
    double px0 = px + aq * y ;
    double py0 = py - aq * x ;
    double phi0    = atan2(py0, px0) ;

    // now d0
    double pt02 = px0 * px0 + py0 * py0 ;
    double pt0 = sqrt(pt02) ;
    double d0 = (pt0 - pt) / aq ;

    // now z0
    double deltaphi = phi - phi0 ;
    if (deltaphi >  TMath::Pi()) deltaphi -= TMath::TwoPi() ;
    else if (deltaphi < -TMath::Pi()) deltaphi += TMath::TwoPi() ;
    double l  = deltaphi / omega ;
    double z0 = z - tandip * l ;

    // now L
    double p2 = pt2 + pz * pz ;
    double p  = sqrt(p2) ;
    double L = l * p / pt ;

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

    double dd0dpx = (dpt0dpx - dptdpx) / aq ;
    double dd0dpy = (dpt0dpy - dptdpy) / aq ;
    double dd0dx  = dpt0dx / aq ;
    double dd0dy  = dpt0dy / aq ;

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
    if (helixpar.num_row() != 6) helixpar = HepVector(6) ;
    helixpar[ex_d0]     = d0 ;
    helixpar[ex_phi0]   = phi0 ;
    helixpar[ex_omega]  = omega ;
    helixpar[ex_z0]     = z0 ;
    helixpar[ex_tanDip] = tandip ;
    helixpar[ex_flt]    = L ;

    // the row is helixpar, the column the vertexpar
    //if(jacobian.num_col()!=6 || jacobian.num_row()!=6)
    //jacobian = HepMatrix(6,6) ;

    if (jacobian.num_col() == 6 && jacobian.num_row() == 6) {
      for (int row = 0; row < 6; ++row)
        for (int col = 0; col < 6; ++col)
          jacobian[row][col] = 0 ;

      jacobian[ex_omega][in_x] = 0 ;
      jacobian[ex_omega][in_y] = 0 ;
      jacobian[ex_omega][in_z] = 0 ;
      jacobian[ex_omega][in_px] = domegadpx ;
      jacobian[ex_omega][in_py] = domegadpy ;
      jacobian[ex_omega][in_pz] = 0 ;

      jacobian[ex_phi0][in_x]  = dphi0dx ;
      jacobian[ex_phi0][in_y]  = dphi0dy ;
      jacobian[ex_phi0][in_z]  = 0 ;
      jacobian[ex_phi0][in_px] = dphi0dpx ;
      jacobian[ex_phi0][in_py] = dphi0dpy ;
      jacobian[ex_phi0][in_pz] = 0 ;

      jacobian[ex_d0][in_x]  = dd0dx ;
      jacobian[ex_d0][in_y]  = dd0dy ;
      jacobian[ex_d0][in_z]  = 0 ;
      jacobian[ex_d0][in_px] = dd0dpx ;
      jacobian[ex_d0][in_py] = dd0dpy ;
      jacobian[ex_d0][in_pz] = 0 ;

      jacobian[ex_tanDip][in_x] = 0 ;
      jacobian[ex_tanDip][in_y] = 0 ;
      jacobian[ex_tanDip][in_z] = 0 ;
      jacobian[ex_tanDip][in_px] = dtandipdpx ;
      jacobian[ex_tanDip][in_py] = dtandipdpy ;
      jacobian[ex_tanDip][in_pz] = dtandipdpz ;

      jacobian[ex_z0][in_x]  = dz0dx ;
      jacobian[ex_z0][in_y]  = dz0dy ;
      jacobian[ex_z0][in_z]  = dz0dz ;
      jacobian[ex_z0][in_px] = dz0dpx ;
      jacobian[ex_z0][in_py] = dz0dpy ;
      jacobian[ex_z0][in_pz] = dz0dpz ;

      jacobian[ex_flt][in_x]  = dLdx ;
      jacobian[ex_flt][in_y]  = dLdy ;
      jacobian[ex_flt][in_z]  = 0 ;
      jacobian[ex_flt][in_px] = dLdpx ;
      jacobian[ex_flt][in_py] = dLdpy ;
      jacobian[ex_flt][in_pz] = dLdpz ;
    }
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

  void HelixUtils::printHelixPar(const HepVector& helixpar)
  {
    for (int i = 0; i < 6; ++i)
      cout << helixParName(i + 1).c_str() << helixpar[i] << endl ;
  }

  void HelixUtils::printVertexPar(const HepVector& vertexpar, int charge)
  {
    for (int i = 0; i < 6; ++i)
      cout << vertexParName(i + 1).c_str() << vertexpar[i] << endl ;
    cout << "charge:    " << charge << endl ;
  }

  void HelixUtils::helixFromVertexNumerical(const HepVector& vertexpar, int charge,
                                            double Bz,
                                            HepVector& helixpar, HepMatrix& jacobian)
  {
    // first call with dummy jacobian
    HepMatrix dummy ;
    HelixUtils::helixFromVertex(vertexpar, charge, Bz, helixpar, dummy) ;

    // numeric calculation of the jacobian
    HepVector vertexpartmp(6) ;
    HepVector helixpartmp(6) ;
    HepMatrix jacobiantmp(6, 6) ;

    for (int jin = 0; jin < 6; ++jin) {
      //double delta = 0.001*abs(vertexpar[jin]) ;
      //if(delta < 1e-8) delta = 1e-8 ;
      double delta = 1.e-5 ;// this is quite a random choice. must change.

      vertexpartmp = vertexpar ;
      vertexpartmp[jin] += delta ;
      HelixUtils::helixFromVertex(vertexpartmp, charge, Bz, helixpartmp, jacobiantmp) ;
      for (int iex = 0; iex < 6; ++iex)
        jacobian[iex][jin] = (helixpartmp[iex] - helixpar[iex]) / delta ;
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

  double HelixUtils::helixPoca(const HepVector& helixpar1,
                               const HepVector& helixpar2,
                               double& flt1, double& flt2,
                               TVector3& vertex, bool parallel)
  {
    double d0_1     = helixpar1[ex_d0]     ;
    double phi0_1   = helixpar1[ex_phi0]   ;
    double omega_1  = helixpar1[ex_omega]  ;
    double z0_1     = helixpar1[ex_z0]     ;
    double tandip_1 = helixpar1[ex_tanDip] ;
    double cosdip_1 = cos(atan(tandip_1))  ; // can do that faster

    double d0_2     = helixpar2[ex_d0]     ;
    double phi0_2   = helixpar2[ex_phi0]   ;
    double omega_2  = helixpar2[ex_omega]  ;
    double z0_2     = helixpar2[ex_z0]     ;
    double tandip_2 = helixpar2[ex_tanDip] ;
    double cosdip_2 = cos(atan(tandip_2))  ;

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
    //    vertex = HepPoint( 0.5*(x1+x2), 0.5*(y1+y2), 0.5*(z1+z2) ) ;
    return sqrt(sqr(x2 - x1) + sqr(y2 - y1) + sqr(z2 - z1)) ;
  }


  double HelixUtils::helixPoca(const HepVector& helixpar,
                               const TVector3& point,
                               double& flt)
  {

    double d0     = helixpar[ex_d0]     ;
    double phi0   = helixpar[ex_phi0]   ;
    double omega  = helixpar[ex_omega]  ;
    double z0     = helixpar[ex_z0]     ;
    double tandip = helixpar[ex_tanDip] ;
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
}
