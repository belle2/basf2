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

#define MUID_MUON 0
#define MUID_PION 1
#define MUID_KAON 2

#define MUID_MaxRange 16
#define MUID_MaxReducedChiSquared 50
#define MUID_ReducedChiSquaredLimit 10.0

  //! Provides muid parameters (from Gearbox)
  class MuidPar {

  public:

    //! Constructor with arguments (experiment #, particleID hypothesis)
    MuidPar(int, const char*);

    //! Destructor
    ~MuidPar();

    //! Get the PDF for a particular hypothesis
    double getPDF(int, int, int, double) const;

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

    //! Reduced chi-squared (transverse) probability density function
    //! with one extra bin for all overflows
    double m_ReducedChiSquaredPDF[4][MUID_MaxReducedChiSquared + 1];

    //! First derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD1[4][MUID_MaxReducedChiSquared + 1];

    //! Second derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD2[4][MUID_MaxReducedChiSquared + 1];

    //! Third derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD3[4][MUID_MaxReducedChiSquared + 1];

    //! Normalization for each reduced chi-squared PDF slice
    double m_ReducedChiSquaredNorm[4][15];

    //! Status flag to indicate whether the PDF tables have been filled or not
    bool m_IsValid;

  };

} // end of namespace Belle2

#endif // MUIDPAR_H
