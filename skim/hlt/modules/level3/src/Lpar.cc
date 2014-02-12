/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno (original version by Nobuhiko Katayama)  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <cmath>
#include "skim/hlt/modules/level3/Lpar.h"

using namespace Belle2;
using namespace L3;

//
// constants, enums and typedefs
//

//
// static data member definitions
//

const double Lpar::BELLE_ALPHA(222.37606);

//
// constructors and destructor
//
// Lpar::Lpar(double x1, double y1, double x2, double y2, double x3, double y3) {
//   circle(x1, y1, x2, y2, x3, y3);
// }
Lpar::Cpar::Cpar(const Lpar& l)
{
  m_cu = l.kappa();
  if (l.alpha() != 0 && l.beta() != 0)
    m_fi = atan2(l.alpha(), -l.beta());
  else m_fi = 0;
  if (m_fi < 0) m_fi += 2 * M_PI;
  m_da = 2 * l.gamma() / (1 + sqrt(1 + 4 * l.kappa() * l.gamma()));
  m_cfi = cos(m_fi);
  m_sfi = sin(m_fi);
}

// Lpar::Lpar( const Lpar& )
// {
// }

Lpar::~Lpar()
{
}

//
// assignment operators
//
// const Lpar& Lpar::operator=( const Lpar& )
// {
// }

//
// comparison operators
//
// bool Lpar::operator==( const Lpar& ) const
// {
// }

// bool Lpar::operator!=( const Lpar& ) const
// {
// }

//
// member functions
//
void Lpar::circle(double x1, double y1, double x2, double y2,
                  double x3, double y3)
{
  double a;
  double b;
  double c;
  double delta = (x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3);
  if (delta == 0) {
    //
    // three points are on a line.
    //
    m_kappa = 0;
    double r12sq = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
    if (r12sq > 0) {
      double r12 = sqrt(r12sq);
      m_beta = -(x1 - x2) / r12;
      m_alpha = (y1 - y2) / r12;
      m_gamma = - (m_alpha * x1 + m_beta * y1);
    } else {
      double r13sq = (x1 - x3) * (x1 - x3) + (y1 - y3) * (y1 - y3);
      if (r13sq > 0) {
        double r13 = sqrt(r13sq);
        m_beta = -(x1 - x3) / r13;
        m_alpha = (y1 - y3) / r13;
        m_gamma = - (m_alpha * x3 + m_beta * y3);
      } else {
        double r23sq = (x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3);
        if (r23sq > 0) {
          double r23 = sqrt(r23sq);
          m_beta = -(x2 - x3) / r23;
          m_alpha = (y2 - y3) / r23;
          m_gamma = - (m_alpha * x3 + m_beta * y3);
        } else {
          m_alpha = 1;
          m_beta = 0;
          m_gamma = 0;
        }
      }
    }
  } else {
    double r1sq = x1 * x1 + y1 * y1;
    double r2sq = x2 * x2 + y2 * y2;
    double r3sq = x3 * x3 + y3 * y3;
    a = 0.5 * ((y1 - y3) * (r1sq - r2sq) - (y1 - y2) * (r1sq - r3sq)) / delta;
    b = 0.5 * (- (x1 - x3) * (r1sq - r2sq) + (x1 - x2) * (r1sq - r3sq)) / delta;
    double csq = (x1 - a) * (x1 - a) + (y1 - b) * (y1 - b);
    c = sqrt(csq);
    //double csq2 = (x2-a)*(x2-a) + (y2-b)*(y2-b);
    //double csq3 = (x3-a)*(x3-a) + (y3-b)*(y3-b);
    m_kappa = 1 / (2 * c);
    m_alpha = - 2 * a * m_kappa;
    m_beta = - 2 * b * m_kappa;
    m_gamma = (a * a + b * b - c * c) * m_kappa;
  }
}

TMatrixD Lpar::dldc() const
#ifdef BELLE_OPTIMIZED_RETURN
return vret(3, 4);
{
#else
{
  TMatrixD vret(3, 4);
#endif
  Cpar cp(*this);
  double xi = cp.xi();
  double s = cp.sfi();
  double c = cp.cfi();
  vret(0, 0) = 2 * cp.da() * s;
  vret(0, 1) = -2 * cp.da() * c;
  vret(0, 2) = cp.da() * cp.da();
  vret(0, 3) = 1;
  vret(1, 0) = xi * c;
  vret(1, 1) = xi * s;
  vret(1, 2) = 0;
  vret(1, 3) = 0;
  vret(2, 0) = 2 * cp.cu() * s;
  vret(2, 1) = -2 * cp.cu() * c;
  vret(2, 2) = xi;
  vret(2, 3) = 0;
  return vret;
}

bool Lpar::xy(double r, double& x, double& y, int dir) const
{
  double t_kr2g = kr2g(r);
  double t_xi2 = xi2();
  double ro = r * r * t_xi2 - t_kr2g * t_kr2g;
  if (ro < 0) return false;
  double rs = sqrt(ro);
  if (dir == 0) {
    x = (- m_alpha * t_kr2g  -  m_beta * rs) / t_xi2;
    y = (- m_beta  * t_kr2g  + m_alpha * rs) / t_xi2;
  } else {
    x = (- m_alpha * t_kr2g  +  m_beta * rs) / t_xi2;
    y = (- m_beta  * t_kr2g  - m_alpha * rs) / t_xi2;
  }
  return true;
}

double Lpar::x(double r) const
{
  double t_x, t_y;
  xy(r, t_x, t_y);
  return t_x;
}

double Lpar::y(double r) const
{
  double t_x, t_y;
  xy(r, t_x, t_y);
  return t_y;
}

double Lpar::phi(double r, int dir) const
{
  double x, y;
  if (!xy(r, x, y, dir)) return -1;
  double p = atan2(y, x);
  if (p < 0) p += (2 * M_PI);
  return p;
}

void Lpar::xhyh(double x, double y, double& xh, double& yh) const
{
  double ddm = dr(x, y);
  if (ddm == 0) {
    xh = x;
    yh = y;
    return;
  }
  double kdp1 = 1 + 2 * kappa() * ddm;
  xh = x - ddm * (2 * kappa() * x + alpha()) / kdp1;
  yh = y - ddm * (2 * kappa() * y + beta()) / kdp1;
}

double Lpar::s(double x, double y) const
{
  double xh, yh, xx, yy;
  xhyh(x, y, xh, yh);
  double fk = fabs(kappa());
  if (fk == 0) return 0;
  yy = 2 * fk * (alpha() * yh - beta() * xh);
  xx = 2 * kappa() * (alpha() * xh + beta() * yh) + xi2();
  double sp = atan2(yy, xx);
  if (sp < 0) sp += (2 * M_PI);
  return sp / 2 / fk;
}

double Lpar::s(double r, int dir) const
{
  double d0 = da();
  if (fabs(r) < fabs(d0)) return -1;
  double b = fabs(kappa()) * sqrt((r * r - d0 * d0) / (1 + 2 * kappa() * d0));
  if (fabs(b) > 1) return -1;
  if (dir == 0)return asin(b) / fabs(kappa());
  return (M_PI - asin(b)) / fabs(kappa());
}

TVectorD Lpar::center() const
#ifdef BELLE_OPTIMIZED_RETURN
return v(3);
{
#else
{
  TVectorD v(3);
#endif
  v(0) = xc();
  v(1) = yc();
  v(2) = 0;
  return (v);
}
