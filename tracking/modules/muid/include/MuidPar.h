/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MUIDPAR_H
#define MUIDPAR_H

namespace Belle2 {

//! Maximum layer-difference index for array size
#define MUID_MaxRange 20

//! Maximum reduced-chi-squared index for array size
#define MUID_MaxReducedChiSquared 250

//! Maximum reduced-chi-squared value for transverse scattering
#define MUID_ReducedChiSquaredLimit 50.0

  //! Provides muid parameters (from Gearbox)
  class MuidPar {

  public:

    //! Constructor with arguments (experiment #, particleID hypothesis)
    MuidPar(int, const char*);

    //! Destructor
    ~MuidPar();

    //! Get the PDF for a particular hypothesis
    double getPDF(int, int, int, int, double) const;

  private:

    //! Hidden constructor
    MuidPar();

    //! Hidden copy constructor
    MuidPar(MuidPar&);

    //! Hidden copy assignment
    MuidPar& operator=(const MuidPar&);

    //! Get probability density functions for this particleID hypothesis from Gearbox
    void fillPDFs(int, const char*);

    //! Construct spline interpolation coefficients (first, second, third derivatives)
    void spline(int, double, double*, double*, double*, double*);

    //! Get the longitudinal-coordinate PDF for a particular hypothesis
    double getPDFRange(int, int, int) const;

    //! Get the transverse-coordinate PDF for a particular hypothesis
    double getPDFRchisq(int, int, double) const;

    //! Range-difference (longitudinal) probability density function
    double m_RangePDF[4][15][MUID_MaxRange];

    //! Reduced chi-squared (transverse) probability density function (overflows in last bin)
    double m_ReducedChiSquaredPDF[4][19][MUID_MaxReducedChiSquared];

    //! First derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD1[4][19][MUID_MaxReducedChiSquared];

    //! Second derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD2[4][19][MUID_MaxReducedChiSquared];

    //! Third derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD3[4][19][MUID_MaxReducedChiSquared];

    //! Status flag to indicate whether the PDF tables have been filled or not
    bool m_IsValid;

  };

} // end of namespace Belle2

#endif // MUIDPAR_H
