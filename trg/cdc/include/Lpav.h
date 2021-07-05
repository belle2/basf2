/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description :
//-----------------------------------------------------------------------------

// system include files

#include <iosfwd>

#include "CLHEP/Matrix/SymMatrix.h"
#include "trg/cdc/Lpar.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCLpav TRGCDCLpav
#endif

namespace Belle2 {

  /// TRGCDCLpav class
  class TRGCDCLpav : public TRGCDCLpar {
    // friend classes and functions

  public:
    // constants, enums and typedefs

    /// Constructor
    TRGCDCLpav();
    /// Destructor
    virtual ~TRGCDCLpav();

    /// member functions for calculation
    void calculate_average(void);
    /// member functions for calculation
    void calculate_average3(void);
    /// member functions for calculation
    void calculate_average(double x, double y, double w = 1);
    /// member functions for calculation
    void calculate_average3(double x, double y, double w = 1);
    /// member functions for calculation
    double calculate_lpar(void);
    /// member functions for calculation
    double calculate_lpar3(void);
    /// member functions for fit
    double fit();
    /// member functions for fit
    double fit(double x, double y, double w = 1);
    /// member functions for clear
    inline void clear();
    /// member functions to add point
    void add_point(double x, double y, double w = 1);
    /// member functions to add point
    void add_point_frac(double x, double y, double w, double f);

    /// const member function nc
    double nc() const { return m_nc; }
    /// const member function cov
    CLHEP::HepSymMatrix cov(int = 0) const;
    /// const member function cov_c
    CLHEP::HepSymMatrix cov_c(int = 0) const;
    /// const member function for extrapolation
    int extrapolate(double, double&, double&) const;
    /// const member function similarity
    double similarity(double, double) const;
    /// const member function for delta chisq
    double delta_chisq(double x, double y, double w = 1) const;
    /// get chisq
    double chisq() const { return m_chisq; }
    /// const member function prob
    double prob() const;
    /// const member function chi_deg
    double chi_deg() const;

    // static member functions

    /// assignment operator(s)
    inline TRGCDCLpav& operator=(const TRGCDCLpav&);
    /// assignment operator(s)
    const TRGCDCLpav& operator=(const TRGCDCLpar&);
    /// assignment operator(s)
    const TRGCDCLpav& operator+=(const TRGCDCLpav&);

    /// ostream operator
    friend std::ostream& operator<<(std::ostream& o, const TRGCDCLpav& s);
    /// + operator
    friend TRGCDCLpav operator+(const TRGCDCLpav&, const TRGCDCLpav&);
    /// exception class, no covarience matrix.
    class Singular {};
    /// exception class, no covarience matrix_c
    class Singular_c {};

  protected:
    // protected member functions

    // protected const member functions

  private:
    /// Constructors and destructor
    inline TRGCDCLpav(const TRGCDCLpav&);

    /// comparison operators
    bool operator==(const TRGCDCLpav&) const;
    /// comparison operators
    bool operator!=(const TRGCDCLpav&) const;

    /// private member functions
//    void add(double x, double y, double w = 1, double a = 0, double b = 0);
    /// private member function sub
    void sub(double x, double y, double w = 1, double a = 0, double b = 0);
    /// private member function calculate_average_n
    void calculate_average_n(double xxav, double yyav, double xyav,
                             double xrrav, double yrrav, double rrrrav);
    /// private member function chisq
    double chisq(double chisq) { m_chisq = chisq; return m_chisq; }
    /// private member function nc
    double nc(double nc) { m_nc = nc; return m_nc; }
    /// private member function solve_lambda
    double solve_lambda(void);
    /// private member function solve_lambda3
    double solve_lambda3(void);

    // private const member functions

    /// data members
    double m_wsum;
    /// data members
    double m_xsum;
    /// data members
    double m_ysum;
    /// data members
    double m_xxsum;
    /// data members
    double m_yysum;
    /// data members
    double m_xysum;
    /// data members
    double m_xrrsum;
    /// data members
    double m_yrrsum;
    /// data members
    double m_rrrrsum;

    /// data members
    double m_wsum_temp;
    /// data members
    double m_xav;
    /// data members
    double m_yav;
    /// data members
    double m_xyavp;

    /// data members
    double m_rscale;
    /// data members
    double m_xxavp;
    /// data members
    double m_yyavp;
    /// data members
    double m_xrravp;
    /// data members
    double m_yrravp;
    /// data members
    double m_rrrravp;
    /// data members
    double m_sinrot;
    /// data members
    double m_cosrot;

    /// data members
    double m_nc;
    /// data members
    double m_chisq;

    // static data members

  };

/// inline function definitions
  inline TRGCDCLpav& TRGCDCLpav::operator=(const TRGCDCLpav& lp)
  {
    TRGCDCLpar::operator=(lp);
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

  inline TRGCDCLpav::TRGCDCLpav(const TRGCDCLpav& lp) : TRGCDCLpar(lp)
  {
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
    return;
  }

  inline void TRGCDCLpav::clear()
  {
    m_wsum = m_xsum = m_ysum = m_xxsum = m_yysum = m_xysum
                                                   = m_xrrsum = m_yrrsum = m_rrrrsum  = m_rscale = m_nc = 0;
    m_chisq = -1;
  }

} // namespace Belle2

