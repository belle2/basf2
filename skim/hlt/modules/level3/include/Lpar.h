/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno (original version by Nobuhiko Katayama)  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef L3_LPAR_H
#define L3_LPAR_H

// system include files
#include <cmath>

// user include files
#include "TVector3.h"
#include "TVectorD.h"
//#include "TMatrixD.h"

namespace Belle2 {
  namespace L3 {

    //! Parameter class for Lpav
    class Lpar {

    public:
      //! Constructor
      Lpar();

      //! Destructor
      virtual ~Lpar();

      //! assignment operator(s)
      Lpar& operator=(const Lpar&);

      //! invert parameter
      void neg();

      //! make circle by three points
      void circle(double x1, double y1, double x2, double y2,
                  double x3, double y3);

      //! returns radius of the circle
      double radius() const { return 0.5 / std::fabs(m_kappa);}

      //! returns center(x) of the circle
      double xc() const { return - m_alpha / 2 / m_kappa; }

      //! returns center(y) of the circle
      double yc() const { return - m_beta / 2 / m_kappa; }

      //! returns path length
      double s(double x, double y) const;

      //! returns path length
      double s(double r, int dir = 0) const;

      //! returns signed distance from the circle
      double d(double x, double y) const;

      //! returns distance from the circle
      double dr(double x, double y) const;

      //! returns phi
      double phi(double r, int dir = 0) const;

      //! returns path length and distance from the circle
      int sd(double r, double x, double y,
             double limit, double& s, double& d) const;

      //! returns Helix parameter at "pivot"
      TVectorD Hpar(const TVector3& pivot) const;

    protected:
      //! Constructors and destructor
      Lpar(const Lpar&);

      //! scale the parameters
      void scale(double s) { m_kappa /= s; m_gamma *= s; }

      //! rotate the parameters
      void rotate(double c, double s);

      //! move the parameters
      void move(double x, double y);

      //! d0
      double d0(double x, double y) const;

      //! returns x y
      bool xy(double, double&, double&, int dir = 0) const;
      //TMatrixD dldc() const;

      //! alpha
      double m_alpha;
      //! beta
      double m_beta;
      //! gamma
      double m_gamma;
      //! kappa
      double m_kappa;

    private:
      /*
      // Private class cpar
      class Cpar {
      public:
        Cpar(const Lpar&);
        double xi() const { return 1 + 2 * m_cu * m_da; }
        double sfi() const { return m_sfi; }
        double cfi() const { return m_cfi; }
        double da() const { return m_da; }
        double cu() const { return m_cu; }
        double fi() const { return m_fi; }
      private:
        double m_cu;
        double m_fi;
        double m_da;
        double m_sfi;
        double m_cfi;
      };
      */

      //! comparison operator
      bool operator==(const Lpar&) const;

      //! comparison operator
      bool operator!=(const Lpar&) const;

      //! check
      double check() const;
      //! kr2g
      double kr2g(double r) const { return m_kappa * r * r + m_gamma; }
      //! returns x
      double x(double r) const;
      //! returns y
      double y(double r) const;
      //! returns xh and yh
      void xhyh(double x, double y, double& xh, double& yh) const;
      //! returns xi2
      double xi2() const { return 1 + 4 * m_kappa * m_gamma; }
      //! returns r_max
      double r_max() const;
      //! returns da
      double da() const {  return 2 * m_gamma / (std::sqrt(xi2()) + 1); }

      //! magnetic field constant
      static const double c_Belle2ALPHA;
    };

    inline
    Lpar::Lpar() : m_alpha(0), m_beta(1), m_gamma(0), m_kappa(0)
    {
    }

    inline
    Lpar::Lpar(const Lpar& l)
    {
      m_alpha = l.m_alpha;
      m_beta = l.m_beta;
      m_gamma = l.m_gamma;
      m_kappa = l.m_kappa;
    }

    inline
    Lpar&
    Lpar::operator=(const Lpar& l)
    {
      if (this != &l) {
        m_alpha = l.m_alpha;
        m_beta = l.m_beta;
        m_gamma = l.m_gamma;
        m_kappa = l.m_kappa;
      }
      return *this;
    }

    inline
    void
    Lpar::rotate(double c, double s)
    {
      double aLpar =  c * m_alpha + s * m_beta;
      double betar = -s * m_alpha + c * m_beta;
      m_alpha = aLpar;
      m_beta = betar;
    }

    inline
    void
    Lpar::move(double x, double y)
    {
      m_gamma += m_kappa * (x * x + y * y) + m_alpha * x + m_beta * y;
      m_alpha += 2 * m_kappa * x;
      m_beta  += 2 * m_kappa * y;
    }

    inline
    double
    Lpar::check() const
    {
      return m_alpha * m_alpha + m_beta * m_beta - 4 * m_kappa * m_gamma - 1;
    }

    inline
    void
    Lpar::neg()
    {
      m_alpha = -m_alpha;
      m_beta = -m_beta;
      m_gamma = -m_gamma;
      m_kappa = -m_kappa;
    }

    inline
    double
    Lpar::d0(double x, double y) const
    {
      return m_alpha * x + m_beta * y + m_gamma + m_kappa * (x * x + y * y);
    }

    inline
    double
    Lpar::d(double x, double y) const
    {
      const double dd = d0(x, y);
      const double approx_limit = 0.2;
      if (std::fabs(m_kappa * dd) > approx_limit) return -1;
      return dd * (1 - m_kappa * dd);
    }

    inline
    double
    Lpar::dr(double x, double y) const
    {
      const double dx = xc() - x;
      const double dy = yc() - y;
      const double r = 0.5 / std::fabs(m_kappa);
      return std::fabs(std::sqrt(dx * dx + dy * dy) - r);
    }

    inline
    double
    Lpar::r_max() const
    {
      if (m_kappa == 0) return 100000000.0;
      if (m_gamma == 0) return 1 / std::fabs(m_kappa);
      return std::fabs(2 * m_gamma / (std::sqrt(1 + 4 * m_gamma * m_kappa) - 1));
    }

    inline
    int
    Lpar::sd(double r, double x, double y,
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
      if (std::fabs(d) > std::fabs(limit)) return 0;
      d_cross_limit = d * limit;
      if (d_cross_limit > limit * limit) return 0;
      return 1;
    }

    inline
    TVectorD
    Lpar::Hpar(const TVector3& pivot) const
    {
      TVectorD a(5);
      const double dd = d0(pivot.x(), pivot.y());
      a(0) = dd * (m_kappa * dd - 1);
      a(1) = (m_kappa > 0)
             ? std::atan2(yc() - pivot.y(), xc() - pivot.x()) + M_PI
             : std::atan2(pivot.y() - yc(), pivot.x() - xc()) - M_PI;
      a(2) = -2.0 * c_Belle2ALPHA * m_kappa;
      a(3) = 0;
      a(4) = 0;
      return a;
    }

  }
}

#endif
