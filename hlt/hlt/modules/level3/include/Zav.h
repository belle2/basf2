/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno (original version by Nobuhiko Katayama)  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef L3_ZAV_H
#define L3_ZAV_H

#include <iostream>

//#include <TMatrixDSym.h>

namespace Belle2 {
  namespace L3 {

    //! Line fitter in the s-z plane
    class Zav {
    public:
      //! Constructor
      Zav();

      //! Destructor
      ~Zav() {}

      //! set
      void set(const Zav*);

      //! returns chi square
      double chisq() const { return m_chisq; }

      //! add point
      void add(double , double, double);

      //! calculate a and b
      double calculate();

      //! returns a (tanL)
      double a() const { return m_a; }

      //! returns b (dz)
      double b() const { return m_b; }

      //! returns z at the path length s
      double z(double s) const { return m_a * s + m_b; }

      //! z distance from the line
      double d(double s, double z) const { return z - m_a * s - m_b; }

      //! returns number of points
      int nc() const { return m_nc; }

      //! clear data members
      void clear(void);

      //! returns covariance matrix
      //TMatrixDSym cov() const;

    private:
      //! parameter a
      double m_a;
      //! parameter b
      double m_b;
      //! weight
      double m_w;
      //! sum of s
      double m_sav;
      //! sum of ss
      double m_ssav;
      //! sum of z
      double m_zav;
      //! sum of sz
      double m_szav;
      //! sum of zz
      double m_zzav;
      //! chi square
      double m_chisq;
      //! 1/sigma
      double m_sig_inv;
      //! covariance matrix element
      double m_c11;
      //! covariance matrix element
      double m_c21;
      //! covariance matrix element
      double m_c22;
      //! number of points
      int m_nc;
    };

    inline Zav::Zav()
      : m_a(-9999.), m_b(-9999.), m_w(0.), m_sav(0.), m_ssav(0.), m_zav(0.),
        m_szav(0.), m_zzav(0.), m_chisq(-1.), m_sig_inv(0.),
        m_c11(0.), m_c21(0.), m_c22(0.), m_nc(0)
    {
    }

    inline void Zav::clear(void)
    {
      m_w = m_sav = m_ssav = m_zav = m_szav = m_zzav = 0;
      m_chisq = -1;
      m_c22 = m_c21 = m_c11 = m_sig_inv = 0;
      m_nc = 0;
    }

    inline void Zav::set(const Zav* c)
    {
      if (c) {
        m_w = c->m_w;
        m_sav = c->m_sav;
        m_ssav = c->m_ssav;
        m_zav = c->m_zav;
        m_szav = c->m_szav;
        m_zzav = c->m_zzav;
        m_sig_inv = c->m_sig_inv;
        m_c11 = c->m_c11;
        m_c21 = c->m_c21;
        m_c22 = c->m_c22;
        m_nc = c->m_nc;
      } else {
        m_w = m_sav = m_ssav = m_zav = m_szav = m_zzav =
                                                  m_sig_inv = m_c11 = m_c21 = m_c22 = 0.;
        m_nc = 0;
      }
      m_a = m_b = 0;
      m_chisq = -1;
    }

    inline void Zav::add(double s, double z, double w)
    {
      m_w += w;
      double sw = s * w;
      m_sav += sw;
      m_ssav += sw * s;
      double zw = z * w;
      m_zav += zw;
      m_szav += zw * s;
      m_zzav += zw * z;
      m_chisq = -1;
      m_nc++;
    }

    inline double Zav::calculate()
    {
      double sig = m_ssav * m_w - m_sav * m_sav;
      if (sig != 0) {
        m_sig_inv = 1 / sig;
        m_a = (m_szav * m_w - m_sav * m_zav) * m_sig_inv;
        m_b = (m_ssav * m_zav - m_sav * m_szav) * m_sig_inv;
        m_chisq = m_zzav - 2 * m_a * m_szav - 2 * m_b * m_zav + m_a * m_a * m_ssav
                  + m_b * m_b * m_w + 2 * m_a * m_b * m_sav;
        m_c11 = m_w * m_sig_inv;
        m_c21 = - m_sav * m_sig_inv;
        m_c22 = m_ssav * m_sig_inv;
      } else {
        m_sig_inv = 0;
        m_c11 = m_c21 = m_c22 = 0;
        m_chisq = -1;
      }
      if (m_nc == 2) {
        //    cout << "NC=2" << *this << endl;
        m_chisq = 0;
      }
      return m_chisq;
    }
    /*
    inline TMatrixDSym Zav::cov() const
    {
      TMatrixDSym vret(2);
      vret(0, 0) = m_ssav;
      vret(1, 0) = m_sav;
      vret(1, 1) = m_w;
      return vret;
    }
    */
  }

}

#endif /* ZAV */

