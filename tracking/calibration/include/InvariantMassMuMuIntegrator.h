/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



#pragma once


namespace Belle2::InvariantMassMuMuCalib {


  /** The integrator aims to evaluate convolution of PDFgenLevel and resolution function */
  class InvariantMassMuMuIntegrator {

  public:

    /** Init the parameters of the PDF integrator */
    void init(double Mean,
              double Sigma,
              double SigmaK,
              double BMean,
              double BDelta,
              double Tau,
              double SigmaE,
              double Frac,
              double M0,
              double Eps,
              double CC,
              double Slope,
              double X);

    /** evaluate the PDF integrand for given t - the integration variable */
    double eval(double t);

    /** Simple integration of the PDF for a to b based on the Trapezoidal rule (for validation) */
    double integralTrap(double a, double b);

    /** Integration of the PDF which avoids steps and uses variable transformation (Trapezoidal rule as back-end) */
    double integralTrapImp(double Eps, double a);

    /** Integration of the PDF which avoids steps and uses variable transformation (Gauss-Konrod rule as back-end) */
    double integralKronrod(double a);

  private:

    double m_mean = 4;    ///< mean position of the resolution function, i.e. (Gaus+Exp tails) conv Gaus
    double m_sigma  = 30; ///< sigma of the resolution function
    double m_sigmaK = 30; ///< sigma of the gaus in the convolution
    double m_bMean = 0;    ///< (bRight + bLeft)/2 where bLeft, bRight are the transition points between gaus and exp (in sigma)
    double m_bDelta = 2.6; ///< (bRight - bLeft)/2 where bLeft, bRight are the transition points between gaus and exp (in sigma)
    double m_tauL = 60;    ///< 1/slope of the left exponential tail
    double m_tauR = 60;    ///< 1/slope of the right exponential tail

    double m_sigmaE = 30;  ///< sigma of the external gaus
    double m_frac = 0.1;   ///< fraction of events in the external gaus

    double m_m0   = 10500; ///< invariant mass of the collisions
    double m_eps  = 0.01;  ///< cut-off term for the power-spectrum caused by the ISR (in GeV)
    double m_slope = 0.95; ///< power in the power-like spectrum from the ISR

    double m_x    = 10400; ///< the resulting PDF is function of this variable the actual rec-level mass

    double m_C = 16;       ///< the coeficient between part bellow eps and above eps cut-off

  };

}
