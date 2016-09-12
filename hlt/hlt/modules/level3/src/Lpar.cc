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

const double Lpar::c_Belle2ALPHA(222.37606);

Lpar::~Lpar()
{
}

//
// member functions
//
void Lpar::circle(double x1, double y1, double x2, double y2,
                  double x3, double y3)
{
  const double delta = (x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3);
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
    const double r1sq = x1 * x1 + y1 * y1;
    const double r2sq = x2 * x2 + y2 * y2;
    const double r3sq = x3 * x3 + y3 * y3;
    const double a
      = 0.5 * ((y1 - y3) * (r1sq - r2sq) - (y1 - y2) * (r1sq - r3sq)) / delta;
    const double b
      = 0.5 * (- (x1 - x3) * (r1sq - r2sq) + (x1 - x2) * (r1sq - r3sq)) / delta;
    const double csq = (x1 - a) * (x1 - a) + (y1 - b) * (y1 - b);
    const double c = sqrt(csq);
    //double csq2 = (x2-a)*(x2-a) + (y2-b)*(y2-b);
    //double csq3 = (x3-a)*(x3-a) + (y3-b)*(y3-b);
    m_kappa = 1 / (2 * c);
    m_alpha = - 2 * a * m_kappa;
    m_beta = - 2 * b * m_kappa;
    m_gamma = (a * a + b * b - c * c) * m_kappa;
  }
}

/*
Lpar::Cpar::Cpar(const Lpar& l)
{
  m_cu = l.m_kappa;
  if (l.m_alpha != 0 && l.m_beta != 0)
    m_fi = atan2(l.m_alpha, -l.m_beta);
  else m_fi = 0;
  if (m_fi < 0) m_fi += 2 * M_PI;
  m_da = 2 * l.m_gamma / (1 + sqrt(1 + 4 * l.m_kappa * l.m_gamma));
  m_cfi = cos(m_fi);
  m_sfi = sin(m_fi);
}

TMatrixD Lpar::dldc() const
{
  TMatrixD vret(3, 4);
  Cpar cp(*this);
  const double xi = cp.xi();
  const double s = cp.sfi();
  const double c = cp.cfi();
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
*/

bool Lpar::xy(double r, double& x, double& y, int dir) const
{
  const double t_kr2g = kr2g(r);
  const double t_xi2 = xi2();
  const double ro = r * r * t_xi2 - t_kr2g * t_kr2g;
  if (ro < 0) return false;
  const double rs = sqrt(ro);
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
  const double ddm = dr(x, y);
  if (ddm == 0) {
    xh = x;
    yh = y;
    return;
  }
  const double kdp1 = 1 + 2 * m_kappa * ddm;
  xh = x - ddm * (2 * m_kappa * x + m_alpha) / kdp1;
  yh = y - ddm * (2 * m_kappa * y + m_beta) / kdp1;
}

double Lpar::s(double x, double y) const
{
  double xh, yh, xx, yy;
  xhyh(x, y, xh, yh);
  const double fk = fabs(m_kappa);
  if (fk == 0) return 0;
  yy = 2 * fk * (m_alpha * yh - m_beta * xh);
  xx = 2 * m_kappa * (m_alpha * xh + m_beta * yh) + xi2();
  double sp = atan2(yy, xx);
  if (sp < 0) sp += (2 * M_PI);
  return sp / 2 / fk;
}

double Lpar::s(double r, int dir) const
{
  const double d0 = da();
  if (fabs(r) < fabs(d0)) return -1;
  const double b
    = fabs(m_kappa) * sqrt((r * r - d0 * d0) / (1 + 2 * m_kappa * d0));
  if (fabs(b) > 1) return -1;
  if (dir == 0) return asin(b) / fabs(m_kappa);
  return (M_PI - asin(b)) / fabs(m_kappa);
}
