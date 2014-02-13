/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno (original version by Nobuhiko Katayama)  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef L3_LPAV_H
#define L3_LPAV_H

// system include files

// user include files
#include "TMatrixDSym.h"
#include "skim/hlt/modules/level3/Lpar.h"

namespace Belle2 {
  namespace L3 {
    class Lpav : public Lpar {

    public:

      //! Constructor
      Lpav();

      //! Destructor
      virtual ~Lpav();

      //! circle fit
      double fit();

      //! circle fit with a new point
      double fit(double x, double y, double w = 1);

      //! clear variables
      void clear();

      //! add a point
      void add_point(double x, double y, double w = 1);

      //! add a point
      void add_point_frac(double x, double y, double w, double f);

      //! returns number of points
      double nc() const { return m_nc; }

      //! returns chi square
      double chisq() const { return m_chisq; }

      //! returns covariance matrix (4x4)
      TMatrixDSym cov(int = 0) const;

      //! returns covariance matrix (3x3)
      TMatrixDSym cov_c(int = 0) const;

      int extrapolate(double, double&, double&) const;
      double similarity(double, double) const;
      double delta_chisq(double x, double y, double w = 1) const;
      double chi_deg() const;

      // assignment operator(s)
      const Lpav& operator=(const Lpav&);
      const Lpav& operator=(const Lpar&);
      const Lpav& operator+=(const Lpav&);

    private:
      // Constructors and destructor
      Lpav(const Lpav&);

      // comparison operators
      bool operator==(const Lpav&) const;
      bool operator!=(const Lpav&) const;

      // private member functions
      void add(double x, double y, double w = 1, double a = 0, double b = 0);
      void sub(double x, double y, double w = 1, double a = 0, double b = 0);
      void calculate_average(void);
      void calculate_average3(void);
      void calculate_average(double x, double y, double w = 1);
      void calculate_average3(double x, double y, double w = 1);
      void calculate_average_n(double xxav, double yyav, double xyav,
                               double xrrav, double yrrav, double rrrrav);
      double chisq(double chisq) { m_chisq = chisq; return m_chisq; }
      double nc(double nc) { m_nc = nc; return m_nc; }
      double solve_lambda(void);
      double solve_lambda3(void);
      double calculate_lpar(void);
      double calculate_lpar3(void);

      // private const member functions

      // data members
      double m_wsum;
      double m_xsum;
      double m_ysum;
      double m_xxsum;
      double m_yysum;
      double m_xysum;
      double m_xrrsum;
      double m_yrrsum;
      double m_rrrrsum;

      double m_wsum_temp;
      double m_xav;
      double m_yav;
      double m_xyavp;

      double m_rscale;
      double m_xxavp;
      double m_yyavp;
      double m_xrravp;
      double m_yrravp;
      double m_rrrravp;
      double m_sinrot;
      double m_cosrot;

      double m_nc;
      double m_chisq;

      // static data members

    };

    //----------------------------------------------
#ifdef Lpav_NO_INLINE
#define inline
#else
#undef inline
#define Lpav_INLINE_DEFINE_HERE
#endif

#ifdef Lpav_INLINE_DEFINE_HERE

    inline
    Lpav::Lpav()
      : Lpar(),
        m_wsum(0.),
        m_xsum(0.),
        m_ysum(0.),
        m_xxsum(0.),
        m_yysum(0.),
        m_xysum(0.),
        m_xrrsum(0.),
        m_yrrsum(0.),
        m_rrrrsum(0.),
        m_wsum_temp(0.),
        m_xav(0.),
        m_yav(0.),
        m_xyavp(0.),
        m_rscale(0.),
        m_xxavp(0.),
        m_yyavp(0.),
        m_xrravp(0.),
        m_yrravp(0.),
        m_rrrravp(0.),
        m_sinrot(0.),
        m_cosrot(0.),
        m_nc(0.),
        m_chisq(-1.)
    {
    }

    inline
    Lpav::Lpav(const Lpav& lp)
      : Lpar(lp),
        m_wsum(lp.m_wsum),
        m_xsum(lp.m_xsum),
        m_ysum(lp.m_ysum),
        m_xxsum(lp.m_xxsum),
        m_yysum(lp.m_yysum),
        m_xysum(lp.m_xysum),
        m_xrrsum(lp.m_xrrsum),
        m_yrrsum(lp.m_yrrsum),
        m_rrrrsum(lp.m_rrrrsum),
        m_wsum_temp(lp.m_wsum_temp),
        m_xav(lp.m_xav),
        m_yav(lp.m_yav),
        m_xyavp(lp.m_xyavp),
        m_rscale(lp.m_rscale),
        m_xxavp(lp.m_xxavp),
        m_yyavp(lp.m_yyavp),
        m_xrravp(lp.m_xrravp),
        m_yrravp(lp.m_yrravp),
        m_rrrravp(lp.m_rrrravp),
        m_sinrot(lp.m_sinrot),
        m_cosrot(lp.m_cosrot),
        m_nc(lp.m_nc),
        m_chisq(lp.m_chisq)
    {
    }

    inline
    const Lpav&
    Lpav::operator=(const Lpav& lp)
    {
      Lpar::operator=(lp);
      m_wsum = lp.m_wsum;
      m_xsum = lp.m_xsum;
      m_ysum = lp.m_ysum;
      m_xxsum = lp.m_xxsum;
      m_yysum = lp.m_yysum;
      m_xysum = lp.m_xysum;
      m_xrrsum = lp.m_xrrsum;
      m_yrrsum = lp.m_yrrsum;
      m_rrrrsum = lp.m_rrrrsum;

      m_wsum_temp = lp. m_wsum_temp;
      m_xav = lp.m_xav;
      m_yav = lp.m_yav;
      m_xyavp = lp.m_xyavp;

      m_rscale = lp. m_rscale;
      m_xxavp = lp.m_xxavp;
      m_yyavp = lp.m_yyavp;
      m_xrravp = lp.m_xrravp;
      m_yrravp = lp.m_yrravp;
      m_rrrravp = lp.m_rrrravp;
      m_sinrot = lp.m_sinrot;
      m_cosrot = lp.m_cosrot;

      m_nc = lp. m_nc;
      m_chisq = lp.m_chisq;
      return *this;
    }

    inline
    void
    Lpav::clear()
    {
      m_wsum = m_xsum = m_ysum = m_xxsum = m_yysum = m_xysum
                                                     = m_xrrsum = m_yrrsum = m_rrrrsum = m_rscale = m_nc = 0;
      m_chisq = -1;
    }
#endif

#undef inline
  }
}

#endif
