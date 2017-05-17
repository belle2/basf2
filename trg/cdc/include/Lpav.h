//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Lpav.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description :
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

// system include files

#include <iosfwd>

#include "CLHEP/Matrix/SymMatrix.h"
#include "trg/cdc/Lpar.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCLpav TRGCDCLpav
#endif

namespace Belle2 {

  class TRGCDCLpav : public TRGCDCLpar {
    // friend classes and functions

  public:
    // constants, enums and typedefs

    // Constructors and destructor
    TRGCDCLpav();
    virtual ~TRGCDCLpav();

    // member functions
    void calculate_average(void);
    void calculate_average3(void);
    void calculate_average(double x, double y, double w = 1);
    void calculate_average3(double x, double y, double w = 1);
    double calculate_lpar(void);
    double calculate_lpar3(void);
    double fit();
    double fit(double x, double y, double w = 1);
    inline void clear();
    void add_point(double x, double y, double w = 1);
    void add_point_frac(double x, double y, double w, double f);

    // const member functions
    double nc() const { return m_nc; }
    CLHEP::HepSymMatrix cov(int = 0) const;
    CLHEP::HepSymMatrix cov_c(int = 0) const;
    int extrapolate(double, double&, double&) const;
    double similarity(double, double) const;
    double delta_chisq(double x, double y, double w = 1) const;
    double chisq() const { return m_chisq; }
    double prob() const;
    double chi_deg() const;

    // static member functions

    // assignment operator(s)
    inline const TRGCDCLpav& operator=(const TRGCDCLpav&);
    const TRGCDCLpav& operator=(const TRGCDCLpar&);
    const TRGCDCLpav& operator+=(const TRGCDCLpav&);

    friend std::ostream& operator<<(std::ostream& o, const TRGCDCLpav& s);
    friend TRGCDCLpav operator+(const TRGCDCLpav&, const TRGCDCLpav&);
    class Singular {}; // exception class, no covarience matrix.
    class Singular_c {}; // exception class, no covarience matrix_c

  protected:
    // protected member functions

    // protected const member functions

  private:
    // Constructors and destructor
    inline TRGCDCLpav(const TRGCDCLpav&);

    // comparison operators
    bool operator==(const TRGCDCLpav&) const;
    bool operator!=(const TRGCDCLpav&) const;

    // private member functions
    void add(double x, double y, double w = 1, double a = 0, double b = 0);
    void sub(double x, double y, double w = 1, double a = 0, double b = 0);
    void calculate_average_n(double xxav, double yyav, double xyav,
                             double xrrav, double yrrav, double rrrrav);
    double chisq(double chisq) { m_chisq = chisq; return m_chisq; }
    double nc(double nc) { m_nc = nc; return m_nc; }
    double solve_lambda(void);
    double solve_lambda3(void);

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

// inline function definitions
  inline const TRGCDCLpav& TRGCDCLpav::operator=(const TRGCDCLpav& lp)
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

