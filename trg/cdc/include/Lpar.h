/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Lpar.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description :
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <iosfwd>
#include <cmath>

#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/Matrix.h"
#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif

#ifdef TRGCDC_SHORT_NAMES
#define TCLpar TRGCDCLpar
#endif

namespace Belle2 {

// forward declarations

  /// TRGCDCLpar class
  class TRGCDCLpar {
    // friend classes and functions

  public:
    // constants, enums and typedefs

    /// Constructor
    TRGCDCLpar();

    /// Destructor
    virtual ~TRGCDCLpar();

    /// assignment operator(s)
    inline TRGCDCLpar& operator=(const TRGCDCLpar&);

    /// member functions
    inline void neg();
    /// circle
    void circle(double x1, double y1, double x2, double y2,
                double x3, double y3);

    /// const member functions
    double kappa() const { return m_kappa; }
    /// const member functions
    double radius() const { return 0.5 / std::fabs(m_kappa);}
    /// const member functions
    CLHEP::HepVector center() const;
    /// const member functions
    double s(double x, double y) const;
    /// const member functions
    inline double d(double x, double y) const;
    /// const member functions
    inline double dr(double x, double y) const;
    /// const member functions
    double s(double r, int dir = 0) const;
    /// const member functions
    double phi(double r, int dir = 0) const;
    /// const member functions
    inline int sd(double r, double x, double y,
                  double limit, double& s, double& d) const;
    /// const member functions
    inline CLHEP::HepVector Hpar(const HepGeom::Point3D<double>&   pivot) const;

    // static member functions

    /// friend functions and classes
    friend class TRGCDCLpav;
    /// ostream operator
    friend std::ostream& operator<<(std::ostream& o, const TRGCDCLpar&);
    /// intersection
    friend int intersect(const TRGCDCLpar&, const TRGCDCLpar&, CLHEP::HepVector&, CLHEP::HepVector&);

  protected:
    // protected member functions

    // protected const member functions

  private:
    /// Private class cpar
    class Cpar {
    public:
      /// constructor  of Cpar class
      explicit Cpar(const TRGCDCLpar&);
      /// returns parameter of Cpar class
      double xi() const { return 1 + 2 * m_cu * m_da; }
      /// returns parameter of Cpar class
      double sfi() const { return m_sfi; }
      /// returns parameter of Cpar class
      double cfi() const { return m_cfi; }
      /// returns parameter of Cpar class
      double da() const { return m_da; }
      /// returns parameter of Cpar class
      double cu() const { return m_cu; }
      /// returns parameter of Cpar class
      double fi() const { return m_fi; }
    private:
      /// parameter of Cpar class
      double m_cu;
      /// parameter of Cpar class
      double m_fi;
      /// parameter of Cpar class
      double m_da;
      /// parameter of Cpar class
      double m_sfi;
      /// parameter of Cpar class
      double m_cfi;
    };
    friend class TRGCDCLpar::Cpar;
    /// Constructors and destructor
    inline TRGCDCLpar(const TRGCDCLpar&);

    /// comparison operators
    bool operator==(const TRGCDCLpar&) const;
    /// comparison operators
    bool operator!=(const TRGCDCLpar&) const;

    /// private member functions
    void scale(double s) { m_kappa /= s; m_gamma *= s; }
    /// private member functions
    inline void rotate(double c, double s);
    /// private member functions
    inline void move(double x, double y);

    /// private const member functions
    double alpha() const { return m_alpha; }
    /// private const member functions
    double beta() const { return m_beta; }
    /// private const member functions
    double gamma() const { return m_gamma; }
    /// private const member functions
    inline double check() const;
    /// private const member functions
    CLHEP::HepMatrix dldc() const;
    /// private const member functions
    inline double d0(double x, double y) const;
    /// private const member functions
    double kr2g(double r) const { return m_kappa * r * r + m_gamma; }
    /// private const member functions
    double x(double r) const;
    /// private const member functions
    double y(double r) const;
    /// private const member functions
    void xhyh(double x, double y, double& xh, double& yh) const;
    /// private const member functions
    double xi2() const { return 1 + 4 * m_kappa * m_gamma; }
    /// private const member functions
    bool xy(double, double&, double&, int dir = 0) const;
    /// private const member functions
    inline double r_max() const;
    /// private const member functions
    double xc() const { return - m_alpha / 2 / m_kappa; }
    /// private const member functions
    double yc() const { return - m_beta / 2 / m_kappa; }
    /// private const member functions
    double da() const {  return 2 * gamma() / (std::sqrt(xi2()) + 1); }
    /// private const member functions
    inline double arcfun(double xh, double yh) const;

    /// data members
    double m_alpha;
    /// data members
    double m_beta;
    /// data members
    double m_gamma;
    /// data members
    double m_kappa;

    /// belle alpha
    static const double BELLE_ALPHA;

    // static data members

  };

// inline function definitions

// inline TRGCDCLpar::TRGCDCLpar(double a, double b, double k, double g) {
//   m_alpha = a; m_beta = b; m_kappa = k; m_gamma = g;
// }

  inline TRGCDCLpar::TRGCDCLpar()
  {
    m_alpha = 0;
    m_beta = 1;
    m_gamma = 0;
    m_kappa = 0;
  }

  inline TRGCDCLpar::TRGCDCLpar(const TRGCDCLpar& l)
  {
    m_alpha = l.m_alpha;
    m_beta = l.m_beta;
    m_gamma = l.m_gamma;
    m_kappa = l.m_kappa;
  }

  inline TRGCDCLpar& TRGCDCLpar::operator=(const TRGCDCLpar& l)
  {
    if (this != &l) {
      m_alpha = l.m_alpha;
      m_beta = l.m_beta;
      m_gamma = l.m_gamma;
      m_kappa = l.m_kappa;
    }
    return *this;
  }

  inline void TRGCDCLpar::rotate(double c, double s)
  {
    double aTRGCDCLpar =  c * m_alpha + s * m_beta;
    double betar = -s * m_alpha + c * m_beta;
    m_alpha = aTRGCDCLpar;
    m_beta = betar;
  }

  inline void TRGCDCLpar::move(double x, double y)
  {
    m_gamma += m_kappa * (x * x + y * y) + m_alpha * x + m_beta * y;
    m_alpha += 2 * m_kappa * x;
    m_beta  += 2 * m_kappa * y;
  }

  inline double TRGCDCLpar::check() const
  {
    return m_alpha * m_alpha + m_beta * m_beta - 4 * m_kappa * m_gamma - 1;
  }

  inline void TRGCDCLpar::neg()
  {
    m_alpha = -m_alpha;
    m_beta = -m_beta;
    m_gamma = -m_gamma;
    m_kappa = -m_kappa;
  }

  inline double TRGCDCLpar::d0(double x, double y) const
  {
    return m_alpha * x + m_beta * y + m_gamma + m_kappa * (x * x + y * y);
  }

  inline double TRGCDCLpar::d(double x, double y) const
  {
    double dd = d0(x, y);
    const double approx_limit = 0.2;
    if (std::fabs(m_kappa * dd) > approx_limit) return -1;
    return dd * (1 - m_kappa * dd);
  }

  inline double TRGCDCLpar::dr(double x, double y) const
  {
    double dx = xc() - x;
    double dy = yc() - y;
    double r = 0.5 / std::fabs(m_kappa);
    return std::fabs(std::sqrt(dx * dx + dy * dy) - r);
  }

  inline double TRGCDCLpar::r_max() const
  {
    if (m_kappa == 0) return 100000000.0;
    if (m_gamma == 0) return 1 / std::fabs(m_kappa);
    return std::fabs(2 * m_gamma / (std::sqrt(1 + 4 * m_gamma * m_kappa) - 1));
  }

  inline double TRGCDCLpar::arcfun(double xh, double yh) const
  {
    //
    // Duet way of calculating Sperp.
    //
    double r2kap = 2.0 * m_kappa;
    double xi = std::sqrt(xi2());
    double xinv = 1.0 / xi;
    double ar2kap = std::fabs(r2kap);
    double cross = m_alpha * yh - m_beta * xh;
    double a1 = ar2kap * cross * xinv;
    double a2 = r2kap * (m_alpha * xh + m_beta * yh) * xinv + xi;
    if (a1 >= 0 && a2 > 0 && a1 < 0.3) {
      double arg2 = a1 * a1;
      return cross * (1.0 + arg2 * (1. / 6. + arg2 * (3. / 40.))) * xinv;
    } else {
      double at2 = std::atan2(a1, a2);
      if (at2 < 0) at2 += (2 * M_PI);
      return at2 / ar2kap;
    }
  }

  inline int TRGCDCLpar::sd(double r, double x, double y,
                            double limit, double& s, double& d) const
  {
    if ((x * yc() - y * xc())*m_kappa < 0) return 0;
    double dd = d0(x, y);
    d = dd * (1 - m_kappa * dd);
    double d_cross_limit = d * limit;
    if (d_cross_limit < 0 || d_cross_limit > limit * limit) return 0;
    double rc = std::sqrt(m_alpha * m_alpha + m_beta * m_beta) / (2 * m_kappa);
    double rho = 1. / (-2 * m_kappa);
    double cosPhi = (rc * rc + rho * rho - r * r) / (-2 * rc * rho);
    cosPhi = cosPhi > 1.0 ? 1.0 : cosPhi;
    cosPhi = cosPhi < -1.0 ? -1.0 : cosPhi;
    double phi = std::acos(cosPhi);
    s = std::fabs(rho) * phi;
    if (0.0 == phi)return 0;
    d *= r / (std::fabs(rc) * std::sin(phi));
//cnv  if (abs(d) > abs(limit)) return 0;
    if (fabs(d) > fabs(limit)) return 0;
    d_cross_limit = d * limit;
    if (d_cross_limit > limit * limit) return 0;
    return 1;
  }

  inline CLHEP::HepVector TRGCDCLpar::Hpar(const HepGeom::Point3D<double>&   pivot) const
  {
    CLHEP::HepVector a(5);
    double dd = d0(pivot.x(), pivot.y());
    a(1) = dd * (m_kappa * dd - 1);
    a(2) = (m_kappa > 0) ? std::atan2(yc() - pivot.y(), xc() - pivot.x()) + M_PI
           : std::atan2(pivot.y() - yc(), pivot.x() - xc()) - M_PI;
    a(3) = -2.0 * BELLE_ALPHA * m_kappa;
    a(4) = 0;
    a(5) = 0;
    return a;
  }

} // namespace Belle2


