/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "TProfile.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH1D.h"
#include "Math/ChebyshevPol.h"
#include "iostream"


namespace Belle2 {
  namespace CDC {

    /**
     * helper function to initialize xt function with 5th order polynomial + linear.
     */
    // cppcheck-suppress constParameter
    Double_t pol5pol1(Double_t* x, Double_t* par)
    {
      Double_t xx = x[0];
      Double_t x6, x2;
      Double_t f, ctp;
      if (xx < par[6]) {
        f = par[0] + par[1] * xx + par[2] * xx * xx + par[3] * xx * xx * xx + par[4] * xx * xx * xx * xx + par[5] * xx * xx * xx * xx * xx;
      } else {
        x6 = par[6];
        x2 = xx - x6;
        ctp = par[0] + par[1] * x6 + par[2] * x6 * x6 + par[3] * x6 * x6 * x6 + par[4] * x6 * x6 * x6 * x6 + par[5] * x6 * x6 * x6 * x6 *
              x6;
        f = par[7] * x2 + ctp;
      }
      return f;
    }

    /**
     * helper function to initialize xt function with 5th order Chebshev Polynomial + linear.
     */
    // cppcheck-suppress constParameter
    Double_t cheby5pol1(Double_t* x, Double_t* par)
    {
      Double_t xx = x[0];
      Double_t x6, x2;
      Double_t f, ctp;

      if (xx < par[6]) {
        f = ROOT::Math::Chebyshev5(xx, par[0], par[1], par[2], par[3], par[4], par[5]);
      } else {
        x6 = par[6];
        x2 = xx - x6;
        ctp = ROOT::Math::Chebyshev5(x6, par[0], par[1], par[2], par[3], par[4], par[5]);
        f = par[7] * x2 + ctp;
      }
      return f;
    }

    /**
     * Class to perform fitting for each xt function.
     */

    class XTFunction {
    public:
      /**
       * Initialized with TProfile histogram
       */
      explicit XTFunction(TProfile* h1)
      {
        m_h1 = (TProfile*)h1->Clone();
        m_h1->SetDirectory(0);
        if (m_mode == c_Chebyshev) {
          m_fitFunc = new TF1("xtCheb5", cheby5pol1, 0.0, 700, 8);
        } else {
          m_fitFunc = new TF1("xtpol5", pol5pol1, 0.0, 700, 8);
        }
      }
      /**
       * Initialized with TProfile histogram and mode
       */
      XTFunction(TProfile* h1, int mode)
      {
        m_h1 = (TProfile*)h1->Clone();
        m_h1->SetDirectory(0);
        m_mode = mode;
        if (m_mode == c_Chebyshev) {
          m_fitFunc = new TF1("xtCheb5", cheby5pol1, 0.0, 700, 8);
        } else {
          m_fitFunc = new TF1("xtpol5", pol5pol1, 0.0, 700, 8);
        }

      }

      /**
       * Initialized with TH1D histogram and mode
       */
      XTFunction(TH1F* h1, int mode)
      {
        m_h1 = (TProfile*)h1->Clone();
        m_h1->SetDirectory(0);
        m_mode = mode;
        if (m_mode == c_Chebyshev) {
          m_fitFunc = new TF1("xtCheb5", cheby5pol1, 0.0, 700, 8);
        } else {
          m_fitFunc = new TF1("xtpol5", pol5pol1, 0.0, 700, 8);
        }
      }

      /**
       * Copy constructor.
       */
      XTFunction(const XTFunction& x) :
        m_h1(x.m_h1),
        m_mode(x.m_mode),
        m_debug(x.m_debug),
        m_draw(x.m_draw),
        m_bField(x.m_bField),
        m_minRequiredEntry(x.m_minRequiredEntry),
        m_fitflag(x.m_fitflag),
        m_Prob(x.m_Prob),
        m_tmin(x.m_tmin),
        m_tmax(x.m_tmax)
      {
        m_fitFunc = (TF1*) x.m_fitFunc->Clone();
        for (int i = 0; i < 8; ++i) {
          m_XTParam[i] = x.m_XTParam[i];
          m_FittedXTParams[i] = x.m_XTParam[i];
        }
      }

      /**
       *  Assignment operator.
       */
      XTFunction& operator=(const XTFunction& x)
      {
        if (this != &x) {
          m_h1 = x.m_h1;
          m_fitFunc = x.m_fitFunc;
          m_mode = x.m_mode;
          m_debug = x.m_debug;
          m_draw = x.m_draw;
          m_bField = x.m_bField;
          m_minRequiredEntry = x.m_minRequiredEntry;
          m_fitflag = x.m_fitflag;
          m_Prob = x.m_Prob;
          m_tmin = x.m_tmin;
          m_tmax = x.m_tmax;

          for (int i = 0; i < 8; ++i) {
            m_XTParam[i] = x.m_XTParam[i];
            m_FittedXTParams[i] = x.m_XTParam[i];
          }
        }
        return *this;
      }


      /**
       * Set Parameter 6 for polynomia fit.
       */
      void  setP6(double p6)
      {
        m_XTParam[6] = p6;
      }

      /**
       * Is valid.
       */
      bool isValid()
      {
        if (m_fitFunc->IsValid() == true) {
          return true;
        } else {
          return false;
        }
      }

      /**
       * Set XT mode.
       * 1 is 5th order Chebshev polynomial.
       * 0 is 5th order polynomial.
       */
      void setMode(int mode)
      {
        m_mode = mode;
      }
      /**
       * set to use BField
       */
      void setBField(bool bfield) {m_bField = bfield;}

      /**
       * Set Parameters for fit.
       */
      void setXTParams(const double p[8])
      {
        for (int i = 0; i < 8; ++i) {m_XTParam[i] = p[i];}
        m_tmax = p[6] + 50;
      }
      /**
       * Set Initial parameters for fitting
       */
      void setXTParams(double p0, double p1, double p2, double p3,
                       double p4, double p5, double p6, double p7)
      {
        m_XTParam[0] = p0;     m_XTParam[1] = p1;     m_XTParam[2] = p2;
        m_XTParam[3] = p3;     m_XTParam[4] = p4;     m_XTParam[5] = p5;
        m_XTParam[6] = p6;     m_XTParam[7] = p7;
        m_tmax = p6 + 50;
      }

      /**
       * Set Fit range.
       */
      void  setFitRange(double tmin, double tmax)
      {
        m_tmin = tmin;
        m_tmax = tmax;
      }
      /**
       * Set minimum number of entry required for fit.
       */
      void setSmallestEntryRequired(int min)
      {
        m_minRequiredEntry = min;
      }
      /**
       * Set Debug.
       */
      void setDebug(bool debug)
      {
        m_debug = debug;
      }
      /**
       * get fitted flag.
       */
      int getFitStatus()
      {
        return m_fitflag;
      }

      /**
       * Get the chi2 probability.
       */
      double getProb()
      {
        return m_Prob;
      }
      /**
       * get fit parameters.
       */
      void  getFittedXTParams(double pa[8])
      {
        for (int i = 0; i < 8; ++i) {pa[i] = m_FittedXTParams[i];}
      }
      /**
       * Get XT function.
       */
      TF1* getXTFunction()
      {
        return m_fitFunc;
      }
      /**
       * Get histogram.
       */
      TProfile* getFittedHisto() {return m_h1;}

      /**
       * Do fitting.
       */
      void fitXT()
      {
        if (m_mode == c_Polynomial) {
          FitPol5();
        } else if (m_mode == c_Chebyshev) {
          FitChebyshev();
        } else {
          B2ERROR("Undefined fitting function");
        }
      }

      /**
       * Fit xt histogram incase 5th order polynomial is used.
       */
      void FitPol5();
      /**
       * Fit xt histogram incase 5th order Chebeshev polynomial is used.
       */
      void FitChebyshev();
      /**
       * Validate the xt has proper shape.
       * Suppose to be bad xt if |xt(0)| > 0.2.
       */
      bool validate();
    private:

      TProfile* m_h1;  /**< Histogram of xt relation. */
      TF1* m_fitFunc;  /**< Fit function */

      //      TF1* xtpol5 = new TF1("xtpol5", pol5pol1, 0.0, 700, 8);  /**< 5th order polynomial function*/
      //      TF1* xtCheb5 = new TF1("xtCheb5", Cheb5pol1, 0.0, 700, 8); /**< 5th order Cheb. polynomial function*/

      int m_mode = c_Chebyshev; /**< XT mode,  0 is for 5th order polynomial, 1 is Chebshev polynomial.*/
      bool m_debug = true;  /**< Print debug durring fitting or not*/
      bool m_draw = false;  /**< Draw and store png plot of each histo or not*/
      bool m_bField = true; /**< With magnetic field or not*/
      int m_minRequiredEntry = 800; /**< Minimum entry required for each histo. */
      double m_XTParam[8] = {};     /**< Parameter fo xt*/
      double m_FittedXTParams[8] = {}; /**< Fitted parameters */

      /**
       *   Fit Flag
       * =-1: low statitic
       * =1: good
       * =0: Fit failure
       * =2: Error Outer
       * =3: Error Inner part;
       */
      int m_fitflag = 0;
      double m_Prob = 0; /**< Chi2 prob of fitting*/
      double m_tmin = 20; /**< lower boundary of fit range*/
      double m_tmax = m_XTParam[6] + 50; /**< upper boundary of fit range*/
    };

    void XTFunction::FitPol5()
    {
      if (m_mode != c_Polynomial) {
        B2ERROR("Fitting function is wrong");
      }
      double max_dif = 0.12;
      double max_dif2 = 0.05;
      double par[8];
      m_h1->Fit("pol1", "MQ", "", m_tmin, 50);
      TF1* f1 = (TF1*)m_h1->GetFunction("pol1");
      double p0 = f1->GetParameter(0);
      double p1 = f1->GetParameter(1);
      double f10 = f1->Eval(10);
      /****************************/
      int in = 0; /*how many time inner part change fit limit*/
      int out = 0; /*how many time outer part change fit limit*/
      m_fitFunc->SetParameters(p0, p1, 0, 0, 0, 0, m_XTParam[6], 0);
      double p6default = m_XTParam[6];
      if (m_bField) {
        m_fitFunc->SetParLimits(7, 0.0, 0.001);
        m_fitFunc->SetParLimits(1, 0.0, 0.01);
      } else {
        m_fitFunc->SetParLimits(7, 0.0, 0.01);
        m_fitFunc->SetParLimits(1, 0.0, 0.01);
      }

      for (int i = 0; i < 10; ++i) {
        m_fitflag = 1;
        std::cout << "Fitting" << std::endl;
        double stat = m_h1->Fit(m_fitFunc, "M", "0", m_tmin, m_tmax);
        m_fitFunc->GetParameters(par);

        /*Eval outer region,*/
        double fp6 = m_fitFunc->Eval(par[6]);
        double fbehindp6 = m_fitFunc->Eval(par[6] - 12) - 0.01;
        if (fp6 < fbehindp6 || fp6 > 1) { /*may be change to good value*/
          m_fitflag = 2;
          out += 1;
          m_fitFunc->SetParameters(p0, p1, 0, 0, 0, 0, p6default, 0);
          m_fitFunc->SetParLimits(6, p6default - 10,  p6default + 10 - out / 2);
          m_tmax += 2;
          if (m_tmax < p6default + 30) {
            m_tmax = p6default + 30;
          }
        }
        /* EVal inner region*/
        /* compare p0 of linear fit vs p0 of xtfun fit and eval point t=10)*/
        if (fabs(par[0] - p0) > max_dif || fabs(f10 - m_fitFunc->Eval(10)) > max_dif2) {
          m_fitflag = 3;
          if (i == 9) std::cout << "ERROR XT FIT inner part" << std::endl;
          in += 1;
          m_fitFunc->SetParameters(p0, p1, 0, 0, 0, 0, p6default, 0);
          m_fitFunc->SetParLimits(1, 0, 0.08);
          m_tmin -= 0.5;

          if (m_tmin < 14) {
            m_tmin = 14; std::cout << "ERROR: tmin so small, fit iter: " << std::endl;
          }
        }

        if (stat == 0) {
          m_fitflag = 0;
          if (m_debug) {std::cout << "FIT Failure; Layer: " << std::endl;}
        }

        if (m_debug) {printf("P6default= %3.2f, p6 fit = %3.2f", p6default, par[6]);}

        if (m_fitflag == 1) {
          if (m_debug) std::cout << "Fit success" << std::endl;
          m_fitFunc->GetParameters(m_FittedXTParams);
          m_Prob = m_fitFunc->GetProb();
          break;
        }

      } //end loop of fitting
      if (m_debug) B2INFO("Number of failures due to inner (outer) regions " << in << "(" << out << ")");

      if (m_draw) {
        TString hname = m_h1->GetName();
        TString name = hname + ".pdf";
        TCanvas* c1 = new TCanvas("c1", "", 800, 600);
        m_h1->Draw();
        m_fitFunc->SetLineColor(kBlack);
        m_fitFunc->DrawF1(0, 400, "same");
        c1->SaveAs(name);
      }
    }

    bool XTFunction::validate()
    {

      const double p6 = m_fitFunc->GetParameter(6);
      if (fabs(m_fitFunc->Eval(0))  > 0.3) {
        B2WARNING("Bad xt function");
        m_fitflag = 0;
        return false;
      } else if (p6 < 50.0) {
        B2WARNING("Unrealistic p6");
        m_fitflag = 0;
        return false;
      } else {
        return true;
      }
    }

    void XTFunction::FitChebyshev()
    {
      if (m_mode != c_Chebyshev) {
        B2ERROR("Fitting function is wrong");
      }

      if (m_h1->GetEntries() < m_minRequiredEntry) {
        m_fitflag = -1;
        return;
      }
      //  m_tmax = m_XTParam[6] + 100;
      //xtCheb5->SetParameters(0.0, 0.005, 0., 0., 0., 0., m_XTParam[6], 0.001);
      double par[8] = {0.0};
      m_fitFunc->SetParLimits(7, 0., 0.001);
      int fitresult = m_h1->Fit("chebyshev5", "QME", "", m_tmin, m_XTParam[6]);
      if (fitresult >= 0) {
        m_h1->GetFunction("chebyshev5")->GetParameters(par);
        m_fitFunc->SetParameters(par[0], par[1], par[2], par[3], par[4], par[5], m_XTParam[6], 0.000);
      }
      double stat;
      for (int i = 0; i < 10; ++i) {
        stat = m_h1->Fit(m_fitFunc, "MQ", "0", m_tmin, m_tmax);
        if (stat == 0) {
          m_fitFunc->SetParameters(par[0], par[1], par[2], par[3], par[4], par[5], m_XTParam[6] - 20, 0.000);
          m_tmax -= 10;
          continue;
        }
        m_fitFunc->GetParameters(par);
        if (par[1] < 0) { // negative c1
          // std::cout << " neg c1 converted" << std::endl;
          par[1] *= -1.0;
          m_fitFunc->SetParLimits(1, 0., 0.01);
          m_tmin += 10.0;
          continue;
        }

        /*Eval outer region,*/
        double fp6 = m_fitFunc->Eval(par[6]);
        double fbehindp6 = m_fitFunc->Eval(par[6] - 10) - 0.005;
        if (fp6 < fbehindp6 || fp6 > 1) { /*may be change to good value*/
          m_fitflag = 2;
          //      out += 1;
          m_fitFunc->SetParameters(par[0], par[1], par[2], par[3], par[4], par[5], par[6] - 20, 0.000);
          m_fitFunc->SetParLimits(6, par[6] - 50,  par[6] - 10);
          m_tmax -= 10;
          //      if (m_tmax < p6default + 30) {
          //  m_tmax = p6default + 30;
          // }
        } else {
          break;
        }
        //    m_tmax +=10;
        //if (stat != 0) break;
      }
      if (par[1] > 0) {
        m_fitFunc->GetParameters(m_FittedXTParams);
        m_Prob = m_fitFunc->GetProb();
      }
      if (stat == 0)
        m_fitflag = 0;
      else
        m_fitflag = 1;
      //  m_fitFunc->DrawF1(0,400,"same");
      if (m_draw) {
        TString hname = m_h1->GetName();
        TString name = hname + ".pdf";
        TCanvas* c1 = new TCanvas("c1", "", 800, 600);
        m_h1->Draw();
        m_fitFunc->SetLineColor(kBlack);
        m_fitFunc->DrawF1(0, 400, "same");
        c1->SaveAs(name);
      }
    }
  }
}
