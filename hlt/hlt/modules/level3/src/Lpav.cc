/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno (original version by Nobuhiko Katayama)  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cmath>
#include <iostream>

// user include files
#include "skim/hlt/modules/level3/Lpav.h"
#include "framework/logging/Logger.h"

using namespace Belle2;
using namespace L3;

//
// constants, enums and typedefs
//
/*
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
*/
//
// constructors and destructor
//
Lpav::~Lpav()
{
}

//
// member functions
//
void Lpav::calculate_average(double xi, double yi, double wi)
{
  if (m_wsum <= 0) return;
  m_wsum_temp = m_wsum + wi;
  const double rri(xi * xi + yi * yi);
  const double wrri(wi * rri);
  const double wsum_inv(1 / m_wsum_temp);
  m_xav = (m_xsum + wi * xi) * wsum_inv;
  m_yav = (m_ysum + wi * yi) * wsum_inv;

  calculate_average_n((m_xxsum + wi * xi * xi) * wsum_inv,
                      (m_yysum + wi * yi * yi) * wsum_inv,
                      (m_xysum + wi * xi * yi) * wsum_inv,
                      (m_xrrsum + xi * wrri) * wsum_inv,
                      (m_yrrsum + yi * wrri) * wsum_inv,
                      (m_rrrrsum + wrri * rri) * wsum_inv);
}

void Lpav::calculate_average(void)
{
  if (m_wsum <= 0) return;
  m_wsum_temp = m_wsum;
  const double wsum_inv(1 / m_wsum_temp);
  m_xav = m_xsum * wsum_inv;
  m_yav = m_ysum * wsum_inv;

  calculate_average_n(m_xxsum * wsum_inv, m_yysum * wsum_inv,
                      m_xysum * wsum_inv, m_xrrsum * wsum_inv,
                      m_yrrsum * wsum_inv, m_rrrrsum * wsum_inv);
}

void Lpav::calculate_average_n(double xxav, double yyav, double xyav,
                               double xrrav, double yrrav, double rrrrav)
{
  const double xxav_p = xxav - m_xav * m_xav;
  const double yyav_p = yyav - m_yav * m_yav;
  const double xyav_p = xyav - m_xav * m_yav;
  const double rrav_p = xxav_p + yyav_p;

  const double a = std::fabs(xxav_p - yyav_p);
  const double b = 4 * xyav_p * xyav_p;
  const double asqpb = a * a + b;
  const double rasqpb = std::sqrt(asqpb);
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
  const double cos2 = m_cosrot * m_cosrot;
  const double sin2 = m_sinrot * m_sinrot;
  const double cs2 = 2 * m_sinrot * m_cosrot;
  const double rrav_p_inv(1 / rrav_p);
  m_xxavp = (cos2 * xxav_p + cs2 * xyav_p + sin2 * yyav_p) * rrav_p_inv;
  m_yyavp = (cos2 * yyav_p - cs2 * xyav_p + sin2 * xxav_p) * rrav_p_inv;

  const double xav2 = m_xav * m_xav;
  const double yav2 = m_yav * m_yav;
  const double xrrav_p = (xrrav - 2 * xxav * m_xav + xav2 * m_xav -
                          2 * xyav * m_yav + m_xav * yav2) - m_xav * rrav_p;
  const double yrrav_p = (yrrav - 2 * yyav * m_yav + yav2 * m_yav -
                          2 * xyav * m_xav + m_yav * xav2) - m_yav * rrav_p;
  m_xrravp = (m_cosrot * xrrav_p + m_sinrot * yrrav_p) * rrav_p_inv / m_rscale;
  m_yrravp = (- m_sinrot * xrrav_p + m_cosrot * yrrav_p) * rrav_p_inv / m_rscale;

  const double rrav = xxav + yyav;
  const double rrrrav_p = rrrrav
                          - 2 * m_yav * yrrav - 2 * m_xav * xrrav
                          + rrav * (xav2 + yav2)
                          - 2 * m_xav * xrrav_p - xav2 * rrav_p
                          - 2 * m_yav * yrrav_p - yav2 * rrav_p;
  m_rrrravp = rrrrav_p * rrav_p_inv * rrav_p_inv;
  m_xyavp = 0;
}

void Lpav::calculate_average3(double xi, double yi, double wi)
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
  const double wrri(wi * rri);
  m_xrravp = (m_xrrsum + xi * wrri) * wsum_inv;
  m_yrravp = (m_yrrsum + yi * wrri) * wsum_inv;
  m_rrrravp = (m_rrrrsum + rri * wrri) * wsum_inv;
}

void Lpav::calculate_average3(void)
{
  if (m_wsum <= 0) return;
  m_wsum_temp = m_wsum;
  const double wsum_inv(1 / m_wsum_temp);
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

double Lpav::solve_lambda(void)
{
  if (m_rscale <= 0) return -1;
  const double xrrxrr = m_xrravp * m_xrravp;
  const double yrryrr = m_yrravp * m_yrravp;
  const double rrrrm1 = m_rrrravp - 1;
  const double xxyy = m_xxavp * m_yyavp;

  const double c0 =  rrrrm1 * xxyy - xrrxrr * m_yyavp - yrryrr * m_xxavp;
  const double c1 = -rrrrm1 + xrrxrr + yrryrr - 4 * xxyy;
  const double c2 =  4 + rrrrm1 - 4 * xxyy;
  const double c4 = -4;
  //
  //C     COEFFICIENTS OF THE DERIVATIVE - USED IN NEWTON-RAPHSON ITERATIONS
  //
  const double c2d = 2 * c2;
  const double c4d = 4 * c4;
  //
  double lambda = 0;

  const double chiscl = m_wsum_temp * m_rscale * m_rscale;
  const double dlamax = 0.001 / chiscl;
  const int ntry = 5;
  int itry = 0;
  double dlambda = dlamax;
  while (itry < ntry && std::fabs(dlambda) >= dlamax) {
    const double cpoly = c0 + lambda * (c1 + lambda *
                                        (c2 + lambda * lambda * c4));
    const double dcpoly = c1 + lambda * (c2d + lambda * lambda * c4d);
    dlambda = - cpoly / dcpoly;
    lambda += dlambda;
    itry ++;
  }
  lambda = lambda < 0 ? 0 : lambda;
  return lambda;
}

double Lpav::solve_lambda3(void)
{
  if (m_rscale <= 0) return -1;
  const double xrrxrr = m_xrravp * m_xrravp;
  const double yrryrr = m_yrravp * m_yrravp;
  //double rrrrm1 = m_rrrravp - 1;
  //double xxyy = m_xxavp * m_yyavp;

  const double a = m_rrrravp;
  const double b = xrrxrr + yrryrr - m_rrrravp * (m_xxavp + m_yyavp);
  const double c = m_rrrravp * m_xxavp * m_yyavp
                   - m_yyavp * xrrxrr - m_xxavp * yrryrr
                   + 2 * m_xyavp * m_xrravp * m_yrravp - m_rrrravp * m_xyavp * m_xyavp;
  if (c >= 0 && b <= 0) {
    return (-b - std::sqrt(b * b - 4 * a * c)) / 2 / a;
  } else if (c >= 0 && b > 0) {
    B2ERROR("Lpav::solve_lambda3: returning -1");
    return -1;
  } else if (c < 0) {
    return (-b + std::sqrt(b * b - 4 * a * c)) / 2 / a;
  }
  return -1;
}

double Lpav::calculate_lpar(void)
{
  const double lambda = solve_lambda();
  // changed on Oct-13-93
  //  if (lambda<=0) return -1;
  if (lambda < 0) return -1;
  const double h11 = m_xxavp - lambda;
  const double h22 = m_yyavp - lambda;
  if (h11 == 0.0) return -1;
  const double h14 = m_xrravp;
  const double h24 = m_yrravp;
  const double h34 = 1 + 2 * lambda;
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

  //---
  // Transform These into the lab. coordinate system
  //---
  // first get kappa and gamma back to real dimensions
  scale(m_rscale);
  // next rotate alpha and beta
  rotate(m_cosrot, -m_sinrot);
  // then translate by (m_xav, m_yav)
  move(-m_xav, -m_yav);
  if (m_yrravp < 0) neg();
  if (lambda >= 0) m_chisq = lambda * m_wsum_temp * m_rscale * m_rscale;
  return lambda;
}

double Lpav::calculate_lpar3(void)
{
  const double lambda = solve_lambda3();
  // changed on Oct-13-93
  //  if (lambda<=0) return -1;
  if (lambda < 0) return -1;
  const double h11 = m_xxavp - lambda;
  const double h22 = m_yyavp - lambda;
  const double h14 = m_xrravp;
  const double h24 = m_yrravp;
  m_gamma = 0;
  const double h12 = m_xyavp;
  const double det = h11 * h22 - h12 * h12;
  if (det != 0) {
    const double r1 = (h14 * h22 - h24 * h12) / (det);
    const double r2 = (h24 * h11 - h14 * h12) / (det);
    const double kinvsq = r1 * r1 + r2 * r2;
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
  if (lambda >= 0) m_chisq = lambda * m_wsum_temp * m_rscale * m_rscale;
  return lambda;
}

double Lpav::fit(double x, double y, double w)
{
  if (m_nc <= 3) return -1;
  m_chisq = -1;
  if (m_nc < 4) {
    calculate_average3(x, y, w);
    const double q = calculate_lpar3();
    if (q > 0) m_chisq = q * m_wsum_temp * m_rscale * m_rscale;
  } else {
    calculate_average(x, y, w);
    const double q = calculate_lpar();
    if (q > 0) m_chisq = q * m_wsum_temp * m_rscale * m_rscale;
  }
  return m_chisq;
}

double Lpav::fit(void)
{
  if (m_nc <= 3) return -1;
  m_chisq = -1;
  if (m_nc < 4) {
    calculate_average3();
    const double q = calculate_lpar3();
    if (q > 0) m_chisq = q * m_wsum_temp * m_rscale * m_rscale;
  } else {
    calculate_average();
    const double q = calculate_lpar();
    if (q > 0) m_chisq = q * m_wsum_temp * m_rscale * m_rscale;
  }
  return m_chisq;
}
/*
TMatrixDSym Lpav::cov(int inv) const
{
  TMatrixDSym vret(4);
  vret(0, 0) = m_xxsum;
  vret(1, 0) = m_xysum;
  vret(1, 1) = m_yysum;
  vret(2, 0) = m_xsum;
  vret(2, 1) = m_ysum;
  vret(2, 2) = m_wsum;
  vret(3, 0) = m_xrrsum;
  vret(3, 1) = m_yrrsum;
  vret(3, 2) = m_xxsum + m_yysum;
  vret(3, 3) = m_rrrrsum;
  if (inv == 0) {
    //    int i=vret.Inv();
    double d;
    vret.Invert(&d);
    if (d == 0.) {
      B2ERROR("Lpav::cov:could not invert nc=" << m_nc);
      return TMatrixDSym(0);
    }
  }
  return vret;
}

TMatrixDSym Lpav::cov_c(int inv) const
{
  TMatrixDSym vret(cov(1).Similarity(dldc()));
  if (inv == 0) {
    double d;
    vret.Invert(&d);
    if (d == 0.) {
      B2ERROR("Lpav::cov_c:could not invert");
      return TMatrixDSym(0);
    }
  }
  return vret;
}

int Lpav::extrapolate(double r, double& phi, double& dphi) const
{
  double x, y;
  if (m_chisq < 0) return -1;
  if (xy(r, x, y) != 0) return -1;
  phi = std::atan2(y, x);
  if (phi < 0) phi += (2 * M_PI);
  TVectorD v(4);
  v(0) = x;
  v(1) = y;
  v(2) = 1;
  v(3) = r * r;
  double l = cov().Similarity(v);
  if (l > 0) {
    double ls = std::sqrt(l);
    dphi = ls / r;
  } else {
    return -1;
  }
  return 0;
}

double Lpav::similarity(double x, double y) const
{
  if (m_nc <= 3) return -1;
  TVectorD v(4);
  v(0) = x;
  v(1) = y;
  v(2) = 1;
  v(3) = x * x + y * y;
  double l = cov().Similarity(v);
  return (l > 0) ? l : -1.;
}

void Lpav::add(double xi, double yi, double w, double a, double b)
{
  const double wi = err_dis_inv(xi, yi, w, a, b);
  add_point(xi, yi, wi);
}
*/

void Lpav::add_point(double xi, double yi, double wi)
{
  m_wsum += wi;
  m_xsum += wi * xi;
  m_ysum += wi * yi;
  m_xxsum += wi * xi * xi;
  m_yysum += wi * yi * yi;
  m_xysum += wi * xi * yi;
  const double rri = (xi * xi + yi * yi);
  const double wrri = wi * rri;
  m_xrrsum += wrri * xi;
  m_yrrsum += wrri * yi;
  m_rrrrsum += wrri * rri;
  m_nc += 1;
}
/*
void Lpav::add_point_frac(double xi, double yi, double w, double a)
{
  const double wi = w * a;
  m_wsum += wi;
  m_xsum += wi * xi;
  m_ysum += wi * yi;
  m_xxsum += wi * xi * xi;
  m_yysum += wi * yi * yi;
  m_xysum += wi * xi * yi;
  const double rri = (xi * xi + yi * yi);
  const double wrri = wi * rri;
  m_xrrsum += wrri * xi;
  m_yrrsum += wrri * yi;
  m_rrrrsum += wrri * rri;
  m_nc += a;
}

void Lpav::sub(double xi, double yi, double w, double a, double b)
{
  const double wi = err_dis_inv(xi, yi, w, a, b);
  m_wsum -= wi;
  m_xsum -= wi * xi;
  m_ysum -= wi * yi;
  m_xxsum -= wi * xi * xi;
  m_yysum -= wi * yi * yi;
  m_xysum -= wi * xi * yi;
  const double rri = (xi * xi + yi * yi);
  const double wrri = wi * rri;
  m_xrrsum -= wrri * xi;
  m_yrrsum -= wrri * yi;
  m_rrrrsum -= wrri * rri;
  m_nc -= 1;
}
*/

Lpav& Lpav::operator+=(const Lpav& la1)
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
/*
double Lpav::chi_deg() const
{
  if (m_nc <= 3) return -1;
  else return m_chisq / (m_nc - 3);
}

double Lpav::delta_chisq(double x, double y, double w) const
{
  double sim = similarity(x, y);
  if (sim < 0) return -1;
  double d = d0(x, y);
  double delta = d * d * w / (1 + sim * w);
  return delta;
}
*/
