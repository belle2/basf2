// -*- C++ -*-
//
// Package:     <package>
// Module:      Lpar
//
// Description: <one line class summary>
//
// Implimentation:
//     <Notes on implimentation>
//
// Author:      KATAYAMA Nobuhiko
// Created:     Fri Feb  6 10:21:49 JST 1998
// $Id: Lpar.cc 9944 2006-11-29 07:36:07Z katayama $
//
// Revision history
//
// $Log$
// Revision 1.5  2001/12/23 09:59:06  katayama
// removed Strings.h
//
// Revision 1.4  2000/04/13 22:13:26  katayama
// added std:: to cout,cerr,endl,stream etc
//
// Revision 1.3  1999/11/23 07:11:16  katayama
// Avoid conflict.
//
// Revision 1.2  1998/07/27 00:04:10  katayama
// Use BELLE_OPTIMIZED_RETURN instead of GNUG
//
// Revision 1.1.1.1  1998/02/16 12:56:25  katayama
// New trasan/lpar classes
//


#include <iostream>

// system include files
#include <cmath>
// user include files
#include "CLHEP/Vector/Sqr.h"
#include "tracking/modules/trasan/Lpar.h"


namespace Belle {

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
//cnv    double csq2 = (x2-a)*(x2-a) + (y2-b)*(y2-b);
//cnv    double csq3 = (x3-a)*(x3-a) + (y3-b)*(y3-b);
      m_kappa = 1 / (2 * c);
      m_alpha = - 2 * a * m_kappa;
      m_beta = - 2 * b * m_kappa;
      m_gamma = (a * a + b * b - c * c) * m_kappa;
    }
  }

  CLHEP::HepMatrix Lpar::dldc() const
#ifdef BELLE_OPTIMIZED_RETURN
  return vret(3, 4);
  {
#else
  {
    CLHEP::HepMatrix vret(3, 4);
#endif
    Cpar cp(*this);
    double xi = cp.xi();
    double s = cp.sfi();
    double c = cp.cfi();
    vret(1, 1) = 2 * cp.da() * s;
    vret(1, 2) = -2 * cp.da() * c;
    vret(1, 3) = cp.da() * cp.da();
    vret(1, 4) = 1;
    vret(2, 1) = xi * c;
    vret(2, 2) = xi * s;
    vret(2, 3) = 0;
    vret(2, 4) = 0;
    vret(3, 1) = 2 * cp.cu() * s;
    vret(3, 2) = -2 * cp.cu() * c;
    vret(3, 3) = xi;
    vret(3, 4) = 0;
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

  CLHEP::HepVector Lpar::center() const
#ifdef BELLE_OPTIMIZED_RETURN
  return v(3);
  {
#else
  {
    CLHEP::HepVector v(3);
#endif
    v(1) = xc();
    v(2) = yc();
    v(3) = 0;
    return(v);
  }

  int intersect(const Lpar& lp1, const Lpar& lp2, CLHEP::HepVector& v1, CLHEP::HepVector& v2)
  {
    CLHEP::HepVector cen1(lp1.center());
    CLHEP::HepVector cen2(lp2.center());
    double dx = cen1(1) - cen2(1);
    double dy = cen1(2) - cen2(2);
    double dc = sqrt(dx * dx + dy * dy);
    if (dc < fabs(0.5 / lp1.kappa()) + fabs(0.5 / lp2.kappa())) {
      double a1 = sqr(lp1.alpha()) + sqr(lp1.beta());
      double a2 = sqr(lp2.alpha()) + sqr(lp2.beta());
      double a3 = lp1.alpha() * lp2.alpha() + lp1.beta() * lp2.beta();
      double det = lp1.alpha() * lp2.beta() - lp1.beta() * lp2.alpha();
      if (fabs(det) > 1e-12) {
        double c1 = a2 * sqr(lp1.kappa()) + a1 * sqr(lp2.kappa()) -
                    2.0 * a3 * lp1.kappa() * lp2.kappa();
        if (c1 != 0) {
          double cinv = 1.0 / c1;
          double c2 = sqr(a3) - 0.5 * (a1 + a2) - 2.0 * a3 *
                      (lp1.gamma() * lp2.kappa() + lp2.gamma() * lp1.kappa());
          double c3 = a2 * sqr(lp1.gamma()) + a1 * sqr(lp2.gamma()) -
                      2.0 * a3 * lp1.gamma() * lp2.gamma();
          double root = sqr(c2) - 4.0 * c1 * c3;
          if (root >= 0) {
            root = sqrt(root);
            double rad2[2];
            rad2[0] = 0.5 * cinv * (-c2 - root);
            rad2[1] = 0.5 * cinv * (-c2 + root);
            double ab1 = -(lp2.beta() * lp1.gamma() - lp1.beta() * lp2.gamma());
            double ab2 = (lp2.alpha() * lp1.gamma() - lp1.alpha() * lp2.gamma());
            double ac1 = -(lp2.beta() * lp1.kappa() - lp1.beta() * lp2.kappa());
            double ac2 = (lp2.alpha() * lp1.kappa() - lp1.alpha() * lp2.kappa());
            double dinv = 1.0 / det;
            v1(1) = dinv * (ab1 + ac1 * rad2[0]);
            v1(2) = dinv * (ab2 + ac2 * rad2[0]);
            v1(3) = 0;
            v2(1) = dinv * (ab1 + ac1 * rad2[1]);
            v2(2) = dinv * (ab2 + ac2 * rad2[1]);
            v2(3) = 0;
//cnv   double d1 = lp1.d(v1(1),v1(2));
//cnv   double d2 = lp2.d(v1(1),v1(2));
//cnv   double d3 = lp1.d(v2(1),v2(2));
//cnv   double d4 = lp2.d(v2(1),v2(2));
//cnv          double r = sqrt(rad2[0]);
            Lpar::Cpar cp1(lp1);
            Lpar::Cpar cp2(lp2);
            for (int j = 0; j < 2; j++) {
              double s1, s2;
              if (j == 0) {
                s1 = lp1.s(v1(1), v1(2));
                s2 = lp2.s(v1(1), v1(2));
              } else {
                s1 = lp1.s(v2(1), v2(2));
                s2 = lp2.s(v2(1), v2(2));
              }
//cnv     double phi1 = cp1.fi() + 2 * cp1.cu() * s1;
//cnv     double phi2 = cp2.fi() + 2 * cp2.cu() * s2;
              double f = (1 + 2 * cp1.cu() * cp1.da()) *
                         (1 + 2 * cp2.cu() * cp2.da()) * cos(cp1.fi() - cp2.fi());
              f -= 2 * (lp1.gamma() * lp2.kappa() + lp2.gamma() * lp1.kappa());
//cnv     double cosphi12 = f;
            }
            return 2;
          }
        }
      }
    }
    return 0;
  }

//
// const member functions
//

//
// static member functions
//

  std::ostream& operator<<(std::ostream& o, Lpar& s)
  {
    return o << " al=" << s.m_alpha << " be=" << s.m_beta
           << " ka=" << s.m_kappa << " ga=" << s.m_gamma;
  }

} // namespace Belle

