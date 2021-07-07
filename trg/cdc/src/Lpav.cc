/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cmath>
#include <iostream>

#include "CLHEP/Vector/Sqr.h"
#include "trg/cdc/Lpav.h"

namespace Belle2 {

//
// constants, enums and typedefs
//
  /// prob function
  extern "C" {
    float prob_(float*, int*);
  }

  /// distance error
  static double err_dis_inv(double x, double y, double w, double a, double b)
  {
    if (a == 0 && b == 0) {
      return w;
    } else {
      double f = x * b - y * a;
      double rsq = x * x + y * y;
      f *= f;
      return w * rsq / f;
    }
  }

//
// static data member definitions
//

//
// constructors and destructor
//
  TRGCDCLpav::TRGCDCLpav() :
    m_wsum(),
    m_xsum(),
    m_ysum(),
    m_xxsum(),
    m_yysum(),
    m_xysum(),
    m_xrrsum(),
    m_yrrsum(),
    m_rrrrsum(),
    m_wsum_temp(),
    m_xav(),
    m_yav(),
    m_xyavp(),
    m_rscale(),
    m_xxavp(),
    m_yyavp(),
    m_xrravp(),
    m_yrravp(),
    m_rrrravp(),
    m_sinrot(),
    m_cosrot(),
    m_nc(),
    m_chisq() // 2019/07/31 by ytlai
  {
    clear();
  }

// TRGCDCLpav::TRGCDCLpav( const TRGCDCLpav& )
// {
// }

  TRGCDCLpav::~TRGCDCLpav()
  {
  }

//
// assignment operators
//
// const TRGCDCLpav& TRGCDCLpav::operator=( const TRGCDCLpav& )
// {
// }

//
// comparison operators
//
// bool TRGCDCLpav::operator==( const TRGCDCLpav& ) const
// {
// }

// bool TRGCDCLpav::operator!=( const TRGCDCLpav& ) const
// {
// }

//
// member functions
//
  void TRGCDCLpav::calculate_average(double xi, double yi, double wi)
  {
    if (m_wsum <= 0) return;
    m_wsum_temp = m_wsum + wi;
    double rri(xi * xi + yi * yi);
    double wrri(wi * rri);
    double wsum_inv(1 / m_wsum_temp);
    m_xav = (m_xsum + wi * xi) * wsum_inv;
    m_yav = (m_ysum + wi * yi) * wsum_inv;

    double xxav((m_xxsum + wi * xi * xi) * wsum_inv);
    double yyav((m_yysum + wi * yi * yi) * wsum_inv);
    double xyav((m_xysum + wi * xi * yi) * wsum_inv);
    double xrrav((m_xrrsum + xi * wrri) * wsum_inv);
    double yrrav((m_yrrsum + yi * wrri) * wsum_inv);
    double rrrrav((m_rrrrsum + wrri * rri) * wsum_inv);

    calculate_average_n(xxav, yyav, xyav, xrrav, yrrav, rrrrav);

  }

  void TRGCDCLpav::calculate_average(void)
  {
    if (m_wsum <= 0) return;
    m_wsum_temp = m_wsum;
    double wsum_inv(1 / m_wsum_temp);
    m_xav = m_xsum * wsum_inv;
    m_yav = m_ysum * wsum_inv;

    double xxav(m_xxsum * wsum_inv);
    double yyav(m_yysum * wsum_inv);
    double xyav(m_xysum * wsum_inv);
    double xrrav(m_xrrsum * wsum_inv);
    double yrrav(m_yrrsum * wsum_inv);
    double rrrrav(m_rrrrsum * wsum_inv);

    calculate_average_n(xxav, yyav, xyav, xrrav, yrrav, rrrrav);
  }

  void TRGCDCLpav::calculate_average_n(double xxav, double yyav, double xyav,
                                       double xrrav, double yrrav, double rrrrav)
  {
    double xxav_p = xxav - m_xav * m_xav;
    double yyav_p = yyav - m_yav * m_yav;
    double xyav_p = xyav - m_xav * m_yav;
    double rrav_p = xxav_p + yyav_p;

    double a = std::fabs(xxav_p - yyav_p);
    double b = 4 * xyav_p * xyav_p;
    double asqpb = a * a + b;
    double rasqpb = std::sqrt(asqpb);
    double splus = 1 + a / rasqpb;
    double sminus = b / (asqpb * splus);
    splus = std::sqrt(0.5 * splus);
    sminus = std::sqrt(0.5 * sminus);
//C
//C== First require : SIGN(C**2 - S**2) = SIGN(XXAV - YYAV)
//C
    if (xxav_p <= yyav_p) {
      m_cosrot = sminus;
      m_sinrot = splus;
    } else {
      m_cosrot = splus;
      m_sinrot = sminus;
    }
//C
//C== Require : SIGN(S) = SIGN(XYAV)*SIGN(C) (Assuming SIGN(C) > 0)
//C
    if (xyav_p < 0) m_sinrot = - m_sinrot;
//*
//* We now have the smallest angle that guarantees <X**2> > <Y**2>
//*
//*  To get the SIGN of the charge right, the new X-AXIS must point
//*  outward from the orgin.  We are free to change signs of both
//*  COSROT and SINROT simultaneously to accomplish this.
//*
//*  Choose SIGN of C wisely to be able to get the sign of the charge
//*
    if (m_cosrot * m_xav + m_sinrot * m_yav <= 0) {
      m_cosrot = - m_cosrot;
      m_sinrot = - m_sinrot;
    }
    m_rscale = std::sqrt(rrav_p);
    double cos2 = m_cosrot * m_cosrot;
    double sin2 = m_sinrot * m_sinrot;
    double cs2 = 2 * m_sinrot * m_cosrot;
    double rrav_p_inv(1 / rrav_p);
    m_xxavp = (cos2 * xxav_p + cs2 * xyav_p + sin2 * yyav_p) * rrav_p_inv;
    m_yyavp = (cos2 * yyav_p - cs2 * xyav_p + sin2 * xxav_p) * rrav_p_inv;

    double xav2 = m_xav * m_xav;
    double yav2 = m_yav * m_yav;
    double xrrav_p = (xrrav - 2 * xxav * m_xav + xav2 * m_xav -
                      2 * xyav * m_yav + m_xav * yav2) - m_xav * rrav_p;
    double yrrav_p = (yrrav - 2 * yyav * m_yav + yav2 * m_yav -
                      2 * xyav * m_xav + m_yav * xav2) - m_yav * rrav_p;
    m_xrravp = (m_cosrot * xrrav_p + m_sinrot * yrrav_p) * rrav_p_inv / m_rscale;
    m_yrravp = (- m_sinrot * xrrav_p + m_cosrot * yrrav_p) * rrav_p_inv / m_rscale;

    double rrav = xxav + yyav;
    double rrrrav_p = rrrrav
                      - 2 * m_yav * yrrav - 2 * m_xav * xrrav
                      + rrav * (xav2 + yav2)
                      - 2 * m_xav * xrrav_p - xav2 * rrav_p
                      - 2 * m_yav * yrrav_p - yav2 * rrav_p;
    m_rrrravp = rrrrav_p * rrav_p_inv * rrav_p_inv;
    m_xyavp = 0;
  }

  void TRGCDCLpav::calculate_average3(double xi, double yi, double wi)
  {
    if (m_wsum <= 0) return;
    m_wsum_temp = m_wsum + wi;
    double wsum_inv(1 / m_wsum_temp);
    double rri(xi * xi + yi * yi);
    m_xav = (m_xsum + wi * xi) * wsum_inv;
    m_yav = (m_ysum + wi * yi) * wsum_inv;

    m_rscale = 1;
    m_cosrot = 1;
    m_sinrot = 0;
    m_xxavp = (m_xxsum + wi * xi * xi) * wsum_inv;
    m_xyavp = (m_xysum + wi * xi * yi) * wsum_inv;
    m_yyavp = (m_yysum + wi * yi * yi) * wsum_inv;
    double wrri(wi * rri);
    m_xrravp = (m_xrrsum + xi * wrri) * wsum_inv;
    m_yrravp = (m_yrrsum + yi * wrri) * wsum_inv;
    m_rrrravp = (m_rrrrsum + rri * wrri) * wsum_inv;
  }

  void TRGCDCLpav::calculate_average3(void)
  {
    if (m_wsum <= 0) return;
    m_wsum_temp = m_wsum;
    double wsum_inv(1 / m_wsum_temp);
    m_xav = m_xsum * wsum_inv;
    m_yav = m_ysum * wsum_inv;

    m_rscale = 1;
    m_cosrot = 1;
    m_sinrot = 0;
    m_xxavp = m_xxsum * wsum_inv;
    m_xyavp = m_xysum * wsum_inv;
    m_yyavp = m_yysum * wsum_inv;
    m_xrravp = m_xrrsum * wsum_inv;
    m_yrravp = m_yrrsum * wsum_inv;
    m_rrrravp = m_rrrrsum * wsum_inv;
  }


//
// const member functions
//

//
// static member functions
//

  /// ostream operator
  std::ostream& operator<<(std::ostream& o, const TRGCDCLpav& a)
  {
//  o << "wsum=" << a.m_wsum << " xsum=" << a.m_xsum << " ysum=" << a.m_ysum
//  << " xxsum=" << a.m_xxsum << " xysum=" << a.m_xysum
//  << " yysum=" << a.m_yysum
//  << " xrrsum=" << a.m_xrrsum << " yrrsum=" << a.m_yrrsum
//  << " rrrrsum=" << a.m_rrrrsum;
//  o << " rscale=" << a.m_rscale
//  << " xxavp=" << a.m_xxavp << " yyavp=" << a.m_yyavp
//  << " xrravp=" << a.m_xrravp << " yrravp=" << a.m_yrravp
//  << " rrrravp=" << a.m_rrrravp << " cosrot=" << a.m_cosrot
//  << " sinrot=" << a.m_sinrot
//  << std::endl;
    o << " nc=" << a.m_nc << " chisq=" << a.m_chisq << " " << (TRGCDCLpar&) a;
    return o;
  }

  double TRGCDCLpav::solve_lambda(void)
  {
    if (m_rscale <= 0) return -1;
    double xrrxrr = m_xrravp * m_xrravp;
    double yrryrr = m_yrravp * m_yrravp;
    double rrrrm1 = m_rrrravp - 1;
    double xxyy = m_xxavp * m_yyavp;

    double c0 =       rrrrm1 * xxyy - xrrxrr * m_yyavp - yrryrr * m_xxavp;
    double c1 =     - rrrrm1        + xrrxrr        + yrryrr        - 4 * xxyy;
    double c2 =   4 + rrrrm1                                        - 4 * xxyy;
    double c4 = - 4;
//
//C     COEFFICIENTS OF THE DERIVATIVE - USED IN NEWTON-RAPHSON ITERATIONS
//
    double c2d = 2 * c2;
    double c4d = 4 * c4;
//
    double lambda = 0;

    double chiscl = m_wsum_temp * m_rscale * m_rscale;
    double dlamax = 0.001 / chiscl;
    const int ntry = 5;
    int itry = 0;
    double dlambda = dlamax;
    while (itry < ntry && std::fabs(dlambda) >= dlamax) {
      double cpoly = c0 + lambda * (c1 + lambda *
                                    (c2 + lambda * lambda * c4));
      double dcpoly = c1 + lambda * (c2d + lambda * lambda * c4d);
      dlambda = - cpoly / dcpoly;
      lambda += dlambda;
      itry ++;
    }
    lambda = lambda < 0 ? 0 : lambda;
    return lambda;
  }

  double TRGCDCLpav::solve_lambda3(void)
  {
    if (m_rscale <= 0) return -1;
    double xrrxrr = m_xrravp * m_xrravp;
    double yrryrr = m_yrravp * m_yrravp;
//cnv  double rrrrm1 = m_rrrravp - 1;
//cnv  double xxyy = m_xxavp * m_yyavp;

    double a = m_rrrravp;
    double b = xrrxrr + yrryrr - m_rrrravp * (m_xxavp + m_yyavp);
    double c = m_rrrravp * m_xxavp * m_yyavp
               - m_yyavp * xrrxrr - m_xxavp * yrryrr
               + 2 * m_xyavp * m_xrravp * m_yrravp - m_rrrravp * m_xyavp * m_xyavp;
    if (c >= 0 && b <= 0) {
      return (-b - std::sqrt(b * b - 4 * a * c)) / 2 / a;
    } else if (c >= 0 && b > 0) {
      std::cout << " returning " << -1 << std::endl;
      return -1;
    } else if (c < 0) {
      return (-b + std::sqrt(b * b - 4 * a * c)) / 2 / a;
    }
    return -1;
  }

  double TRGCDCLpav::calculate_lpar(void)
  {
    double lambda = solve_lambda();
// changed on Oct-13-93
//  if (lambda<=0) return -1;
    if (lambda < 0) return -1;
    double h11 = m_xxavp - lambda;
    double h22 = m_yyavp - lambda;
    if (h11 == 0.0) return -1;
    double h14 = m_xrravp;
    double h24 = m_yrravp;
    double h34 = 1 + 2 * lambda;
    double rootsq = (h14 * h14 / h11 / h11) + 4 * h34;
    if (std::fabs(h22) > std::fabs(h24)) {
      if (h22 == 0.0) return -1;
      double ratio = h24 / h22;
      rootsq += ratio * ratio ;
      m_kappa = 1 / std::sqrt(rootsq);
      m_beta = - ratio * m_kappa;
    } else {
      if (h24 == 0.0) return -1;
      double ratio = h22 / h24;
      rootsq = 1 + ratio * ratio * rootsq;
      m_beta = 1 / std::sqrt(rootsq);
      m_beta = h24 > 0 ? -m_beta : m_beta;
      m_kappa = -ratio * m_beta;
    }
    m_alpha = - (h14 / h11) * m_kappa;
    m_gamma = - h34 * m_kappa;
//    if (lambda<0.0001) {
//      std::cout << " lambda=" << lambda << " h34=" << h34
//  << " rootsq=" << rootsq << " h22=" << h22
//    << " h11=" << h11 << " h14=" << h14 << " h24=" << h24 <<
//      " " << *this << std::endl;
//    }
//
//C     TRANSFORM THESE INTO THE LAB COORDINATE SYSTEM
//
//C     FIRST GET KAPPA  AND GAMMA  BACK TO REAL DIMENSIONS
//
    scale(m_rscale);
//
//C     NEXT ROTATE ALPHA  AND BETA
//
    rotate(m_cosrot, -m_sinrot);
//
//C     THEN TRANSLATE BY (XAV,YAV)
//
    move(-m_xav, -m_yav);
    if (m_yrravp < 0) neg();
    if (lambda >= 0) m_chisq = lambda * m_wsum_temp * m_rscale * m_rscale;
    return lambda;
  }

  double TRGCDCLpav::calculate_lpar3(void)
  {
    double lambda = solve_lambda3();
// changed on Oct-13-93
//  if (lambda<=0) return -1;
    if (lambda < 0) return -1;
    double h11 = m_xxavp - lambda;
    double h22 = m_yyavp - lambda;
    double h14 = m_xrravp;
    double h24 = m_yrravp;
    m_gamma = 0;
    double h12 = m_xyavp;
    double det = h11 * h22 - h12 * h12;
    if (det != 0) {
      double r1 = (h14 * h22 - h24 * h12) / (det);
      double r2 = (h24 * h11 - h14 * h12) / (det);
      double kinvsq = r1 * r1 + r2 * r2;
      m_kappa = std::sqrt(1 / kinvsq);
      if (h11 != 0) m_alpha = -m_kappa * r1;
      else m_alpha = 1;
      if (h22 != 0) m_beta = -m_kappa * r2;
      else m_beta = 1;
    } else {
      m_kappa = 0;
      if (h11 != 0 && h22 != 0) {
        m_beta = 1 / std::sqrt(1 + h12 * h12 / h11 / h11);
        m_alpha = std::sqrt(1 - m_beta * m_beta);
      } else if (h11 != 0) {
        m_beta = 1;
        m_alpha = 0;
      } else {
        m_beta = 0;
        m_alpha = 1;
      }
    }
    if ((m_alpha * m_xav + m_beta * m_yav) *
        (m_beta * m_xav - m_alpha * m_yav) < 0) neg();
//    if (std::fabs(m_alpha)<0.01 && std::fabs(m_beta)<0.01) {
//      std::cout << " lambda=" << lambda << " " << *this << std::endl;
//    }
    if (lambda >= 0) m_chisq = lambda * m_wsum_temp * m_rscale * m_rscale;
    return lambda;
  }

  double TRGCDCLpav::fit(double x, double y, double w)
  {
    if (m_nc <= 3) return -1;
    m_chisq = -1;
    if (m_nc < 4) {
      calculate_average3(x, y, w);
      double q = calculate_lpar3();
      if (q > 0) m_chisq = q * m_wsum_temp * m_rscale * m_rscale;
    } else {
      calculate_average(x, y, w);
      double q = calculate_lpar();
      if (q > 0) m_chisq = q * m_wsum_temp * m_rscale * m_rscale;
    }
    return m_chisq;
  }

  double TRGCDCLpav::fit(void)
  {
    if (m_nc <= 3) return -1;
    m_chisq = -1;
    double q;
    if (m_nc < 4) {
      calculate_average3();
      q = calculate_lpar3();
      if (q > 0) m_chisq = q * m_wsum_temp * m_rscale * m_rscale;
    } else {
      calculate_average();
      q = calculate_lpar();
      if (q > 0) m_chisq = q * m_wsum_temp * m_rscale * m_rscale;
    }
    return m_chisq;
  }

  CLHEP::HepSymMatrix TRGCDCLpav::cov(int inv) const
#ifdef BELLE_OPTIMIZED_RETURN
  return vret(4);
  {
#else
  {
    CLHEP::HepSymMatrix vret(4);
#endif
    vret(1, 1) = m_xxsum;
    vret(2, 1) = m_xysum;
    vret(2, 2) = m_yysum;
    vret(3, 1) = m_xsum;
    vret(3, 2) = m_ysum;
    vret(3, 3) = m_wsum;
    vret(4, 1) = m_xrrsum;
    vret(4, 2) = m_yrrsum;
    vret(4, 3) = m_xxsum + m_yysum;
    vret(4, 4) = m_rrrrsum;
    if (inv == 0)
    {
//    int i=vret.Inv();
      int i;
      vret.invert(i);
      if (i != 0) {
        std::cout << "TRGCDCLpav::cov:could not invert nc=" << m_nc << vret;
#ifdef HAVE_EXCEPTION
        throw new Singular();
#endif
      }
    }
    return vret;
  }

  CLHEP::HepSymMatrix TRGCDCLpav::cov_c(int inv) const
#ifdef BELLE_OPTIMIZED_RETURN
  return vret(3);
  {
#else
  {
    CLHEP::HepSymMatrix vret(3);
#endif
#ifdef HAVE_EXCEPTION
    try {
#endif
      vret = cov(1).similarity(dldc());
#ifdef HAVE_EXCEPTION
    } catch (TRGCDCLpav::Singular)
    {
      throw new Singular_c();
    }
#endif
    if (inv == 0)
    {
//    int i = vret.Inv();
      int i;
      vret.invert(i);
      if (i != 0) {
        std::cout << "TRGCDCLpav::cov_c:could not invert " << vret;
#ifdef HAVE_EXCEPTION
        throw new Singular_c();
#endif
      }
    }
    return vret;
  }

  int TRGCDCLpav::extrapolate(double r, double& phi, double& dphi) const
  {
    double x, y;
    if (m_chisq < 0) return -1;
    if (xy(r, x, y) != 0) return -1;
    phi = std::atan2(y, x);
    if (phi < 0) phi += (2 * M_PI);
    CLHEP::HepVector v(4);
    v(1) = x;
    v(2) = y;
    v(3) = 1;
    v(4) = r * r;
//  CLHEP::HepSymMatrix l = cov().similarityT(v);
#ifdef HAVE_EXCEPTION
    try {
#endif
//     CLHEP::HepSymMatrix l = cov().similarity(v.T());
//     //  std::cout << "delta d^2=" << l(1,1);
//     if (l(1,1)>0) {
      double l = cov().similarity(v);
      if (l > 0) {
        double ls = std::sqrt(l);
        dphi = ls / r;
        //    std::cout << " delta d=" << ls << " dphi=" << dphi;
      }
#ifdef HAVE_EXCEPTION
    } catch (TRGCDCLpav::Singular) {
      return -1;
    }
#endif
//  std::cout << std::endl;
    return 0;
  }

  double TRGCDCLpav::similarity(double x, double y) const
  {
    if (m_nc <= 3) return -1;
    CLHEP::HepVector v(4);
    v(1) = x;
    v(2) = y;
    v(3) = 1;
    v(4) = x * x + y * y;
    double l;
#ifdef HAVE_EXCEPTION
    try {
#endif
      l = cov().similarity(v);
#ifdef HAVE_EXCEPTION
    } catch (TRGCDCLpav::Singular) {
      return -1;
    }
#endif
    return l;
  }

//  void TRGCDCLpav::add(double xi, double yi, double w, double a, double b)
//  {
//double wi = err_dis_inv(xi, yi, w, a, b);
//    add(xi, yi, wi); calling itself with output
//  }
//
  void TRGCDCLpav::add_point(double xi, double yi,
                             double wi)
  {
    m_wsum += wi;
    m_xsum += wi * xi;
    m_ysum += wi * yi;
    m_xxsum += wi * xi * xi;
    m_yysum += wi * yi * yi;
    m_xysum += wi * xi * yi;
//double rri = ( xi * xi + yi * yi );
//double wrri = wi * rri;
    double rri = (xi * xi + yi * yi);
    double wrri = wi * rri;
    m_xrrsum += wrri * xi;
    m_yrrsum += wrri * yi;
    m_rrrrsum += wrri * rri;
    m_nc += 1;
  }

  void TRGCDCLpav::add_point_frac(double xi, double yi, double w, double a)
  {
//double wi = w * a;
    double wi = w * a;
    m_wsum += wi;
    m_xsum += wi * xi;
    m_ysum += wi * yi;
    m_xxsum += wi * xi * xi;
    m_yysum += wi * yi * yi;
    m_xysum += wi * xi * yi;
//double rri = ( xi * xi + yi * yi );
//double wrri = wi * rri;
    double rri = (xi * xi + yi * yi);
    double wrri = wi * rri;
    m_xrrsum += wrri * xi;
    m_yrrsum += wrri * yi;
    m_rrrrsum += wrri * rri;
    m_nc += a;
  }

  void TRGCDCLpav::sub(double xi, double yi, double w, double a, double b)
  {
//double wi = err_dis_inv(xi, yi, w, a, b);
    double wi = err_dis_inv(xi, yi, w, a, b);
    m_wsum -= wi;
    m_xsum -= wi * xi;
    m_ysum -= wi * yi;
    m_xxsum -= wi * xi * xi;
    m_yysum -= wi * yi * yi;
    m_xysum -= wi * xi * yi;
//double rri = ( xi * xi + yi * yi );
//double wrri = wi * rri;
    double rri = (xi * xi + yi * yi);
    double wrri = wi * rri;
    m_xrrsum -= wrri * xi;
    m_yrrsum -= wrri * yi;
    m_rrrrsum -= wrri * rri;
    m_nc -= 1;
  }

  const TRGCDCLpav& TRGCDCLpav::operator+=(const TRGCDCLpav& la1)
  {
    m_wsum += la1.m_wsum;
    m_xsum += la1.m_xsum;
    m_ysum += la1.m_ysum;
    m_xxsum += la1.m_xxsum;
    m_yysum += la1.m_yysum;
    m_xysum += la1.m_xysum;
    m_xrrsum += la1.m_xrrsum;
    m_yrrsum += la1.m_yrrsum;
    m_rrrrsum += la1.m_rrrrsum;
    m_nc += la1.m_nc;
    return *this;
  }

  TRGCDCLpav operator+(const TRGCDCLpav& la1, const TRGCDCLpav& la2)
#ifdef BELLE_OPTIMIZED_RETURN
  return la;
  {
#else
  {
    TRGCDCLpav la;
#endif
    la.m_wsum = la1.m_wsum + la2.m_wsum;
    la.m_xsum = la1.m_xsum + la2.m_xsum;
    la.m_ysum = la1.m_ysum + la2.m_ysum;
    la.m_xxsum = la1.m_xxsum + la2.m_xxsum;
    la.m_yysum = la1.m_yysum + la2.m_yysum;
    la.m_xysum = la1.m_xysum + la2.m_xysum;
    la.m_xrrsum = la1.m_xrrsum + la2.m_xrrsum;
    la.m_yrrsum = la1.m_yrrsum + la2.m_yrrsum;
    la.m_rrrrsum = la1.m_rrrrsum + la2.m_rrrrsum;
    la.m_nc = la1.m_nc + la2.m_nc;
    return la;
  }

  double TRGCDCLpav::prob() const
  {
    if (m_nc <= 3) return 0;
    if (m_chisq < 0) return 0;
//cnv  float c = m_chisq;
//cnv  int nci = (int)m_nc - 3;
    // temporarily commented out
    //  double p = (double) prob_(&c, &nci);
    double p = 0;
    return p;
  }

  double TRGCDCLpav::chi_deg() const
  {
    if (m_nc <= 3) return -1;
    else return m_chisq / (m_nc - 3);
  }

  double TRGCDCLpav::delta_chisq(double x, double y, double w) const
  {
    double sim = similarity(x, y);
    if (sim < 0) return -1;
    double d = d0(x, y);
    double delta = sqr(d) * w / (1 + sim * w);
    return delta;
  }

} // namespace Belle2
