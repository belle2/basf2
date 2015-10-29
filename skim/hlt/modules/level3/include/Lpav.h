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
//#include "TMatrixDSym.h"
#include "skim/hlt/modules/level3/Lpar.h"

namespace Belle2 {
  namespace L3 {

    //! Circle fitter in the r-phi plane
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

      //! returns number of points
      double nc() const { return m_nc; }

      //! returns chi square
      double chisq() const { return m_chisq; }

      /*
      // unused functions
      TMatrixDSym cov(int = 0) const;
      TMatrixDSym cov_c(int = 0) const;
      void add_point_frac(double x, double y, double w, double f);
      int extrapolate(double, double&, double&) const;
      double similarity(double, double) const;
      double delta_chisq(double x, double y, double w = 1) const;
      double chi_deg() const;
      */

      //! assignment operator
      Lpav& operator=(const Lpav&);

      //! assignment operator
      Lpav& operator+=(const Lpav&);

    private:
      //! constructor
      Lpav(const Lpav&);

      //! comparison operator
      bool operator==(const Lpav&) const;

      //! comparison operator
      bool operator!=(const Lpav&) const;

      /*
      // unused functions
      //! add point
      void add(double x, double y, double w = 1, double a = 0, double b = 0);
      //! subtract point
      void sub(double x, double y, double w = 1, double a = 0, double b = 0);
      */

      //! calculate average
      void calculate_average(void);
      //! calculate average
      void calculate_average3(void);
      //! calculate average
      void calculate_average(double x, double y, double w = 1);
      //! calculate average
      void calculate_average3(double x, double y, double w = 1);
      //! calculate average
      void calculate_average_n(double xxav, double yyav, double xyav,
                               double xrrav, double yrrav, double rrrrav);
      //! set chi square
      double chisq(double chisq) { m_chisq = chisq; return m_chisq; }
      //! set number of points
      double nc(double nc) { m_nc = nc; return m_nc; }
      //! solve lambda
      double solve_lambda(void);
      //! solve lambda
      double solve_lambda3(void);
      //! calculate Lpar
      double calculate_lpar(void);
      //! calculate Lpar
      double calculate_lpar3(void);

      //! sum of weight
      double m_wsum;
      //! sum of x
      double m_xsum;
      //! sum of y
      double m_ysum;
      //! sum of x*x
      double m_xxsum;
      //! sum of y*y
      double m_yysum;
      //! sum of x*y
      double m_xysum;
      //! sum of x*r*r
      double m_xrrsum;
      //! sum of y*r*r
      double m_yrrsum;
      //! sum of r*r*r*r
      double m_rrrrsum;

      //! sum of weight?
      double m_wsum_temp;
      //! average of x?
      double m_xav;
      //! average of y?
      double m_yav;
      //! average of xy?
      double m_xyavp;
      //! rscale?
      double m_rscale;
      //! average of xx?
      double m_xxavp;
      //! average of yy?
      double m_yyavp;
      //! average of xrr?
      double m_xrravp;
      //! average of yrr?
      double m_yrravp;
      //! average of rrrr?
      double m_rrrravp;
      //! sin(rot)?
      double m_sinrot;
      //! cos(rot)?
      double m_cosrot;

      //! number of points
      double m_nc;

      //! chi square
      double m_chisq;

    };

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
    Lpav&
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
      m_wsum = m_xsum = m_ysum = m_xxsum = m_yysum = m_xysum = m_xrrsum = m_yrrsum = m_rrrrsum = m_rscale = m_nc = 0;
      m_chisq = -1;
    }
  }
}

#endif
