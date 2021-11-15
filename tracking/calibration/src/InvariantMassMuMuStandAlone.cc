/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <iostream>
#include <iomanip>
#include <vector>
#include <tuple>
#include <numeric>
#include <TTree.h>
#include <TVector3.h>
#include <Eigen/Dense>


#include "TGraph.h"
#include "TH1D.h"
#include "Math/Functor.h"
#include "Math/SpecFuncMathCore.h"
#include "Math/DistFunc.h"
#include "TCanvas.h"
#include <fstream>


//if compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/InvariantMassMuMuStandAlone.h>
#include <tracking/calibration/Splitter.h>
#include <tracking/calibration/tools.h>
#include <tracking/calibration/ChebFitter.h>
#else
#include <InvariantMassMuMuStandAlone.h>
#include <Splitter.h>
#include <tools.h>
#include <ChebFitter.h>
#endif

using Eigen::MatrixXd;
using Eigen::VectorXd;

using namespace std;

namespace Belle2 {

  namespace InvariantMassMuMuCalib {



    /// read events from TTree to std::vector
    vector<Event> getEvents(TTree* tr, bool is4S)
    {

      vector<Event> events;
      events.reserve(tr->GetEntries());

      Event evt;

      tr->SetBranchAddress("run", &evt.run);
      tr->SetBranchAddress("exp", &evt.exp);
      tr->SetBranchAddress("event", &evt.evtNo);

      TVector3* p0 = nullptr;
      TVector3* p1 = nullptr;

      tr->SetBranchAddress("mu0_p", &p0);
      tr->SetBranchAddress("mu1_p", &p1);

      tr->SetBranchAddress("mu0_pid", &evt.mu0.pid);
      tr->SetBranchAddress("mu1_pid", &evt.mu1.pid);


      tr->SetBranchAddress("time", &evt.t); //time in hours

      const double mMu  = 105.6583745e-3; //muon mass [GeV]

      for (int i = 0; i < tr->GetEntries(); ++i) {
        tr->GetEntry(i);

        evt.mu0.p = *p0;
        evt.mu1.p = *p1;

        evt.m = sqrt(pow(hypot(evt.mu0.p.Mag(), mMu) + hypot(evt.mu1.p.Mag(), mMu), 2)   - (evt.mu0.p + evt.mu1.p).Mag2());

        evt.nBootStrap = 1;
        evt.isSig = true;
        evt.is4S  = is4S;
        events.push_back(evt);
      }

      //sort by time
      sort(events.begin(), events.end(), [](Event e1, Event e2) {return e1.t < e2.t;});


      return events;
    }

    // Analysis itself





// integral based on https://en.wikipedia.org/wiki/Gauss%E2%80%93Kronrod_quadrature_formula
// Function evaluated at 15 points chosen in a clever way,
// corresponds to the interpolation by polynomial of order 29
// and calculating it's area
    double integrate(function<double(double)> f, double a, double b)
    {
      static const vector<double> nodes = {
        -0.991455371120813,
        -0.949107912342759,
        -0.864864423359769,
        -0.741531185599394,
        -0.586087235467691,
        -0.405845151377397,
        -0.207784955007898,
        0.000000000000000
      };

      static const vector<double> wgts = {
        0.022935322010529,
        0.063092092629979,
        0.104790010322250,
        0.140653259715525,
        0.169004726639267,
        0.190350578064785,
        0.204432940075298,
        0.209482141084728
      };

      double m = (b + a) / 2; //middle of the interval
      double d = (b - a) / 2; //half-width of the interval

      double sum = 0;
      for (unsigned i = 0; i < nodes.size() - 1; ++i) {
        double x1 = m - d * nodes[i];
        double x2 = m + d * nodes[i];
        sum += (f(x1) + f(x2)) * wgts[i];
      }

      //add the central point (which is not in pair)
      sum += f(m) * wgts.back();


      // scale by size of interval, for example a=-1, b=1 -> no scaling
      return sum * d;
    }










//int a to b of exp(-c x^2 + 2*d -d*d/c)
    double gausInt(double a, double b, double c, double d)
    {
      double res = sqrt(M_PI) / (2 * sqrt(c)) * (TMath::Erf((b * c - d) / sqrt(c)) - TMath::Erf((a * c - d) / sqrt(c)));
      return res;
    }

    double convGausGaus(double sK, double s, double a, double b, double m, double x)
    {
      a -= m;
      b -= m;
      x -= m;


      double c = 1. / 2 * (1. / s / s + 1. / sK / sK);
      double d = 1. / 2 * x / sK / sK;

      //double Const = 1./(2*M_PI) * 1./(s*sK)  * exp(-1./2*pow(x/sK,2) + d*d/c);
      double Const = 1. / (2 * M_PI) * 1. / (s * sK)  * exp(-1. / 2 * x * x / (s * s + sK * sK));

      double res = Const * gausInt(a, b, c, d);
      assert(isfinite(res));
      return res;
    }





    double convExpGaus(double sK, double tau, double a, double x)
    {
      x -= a;

      /*
      double c = 1./(2*sK*sK);
      double d = 0.5*(x/sK/sK - 1/tau);

      //double Const = 1./(sqrt(2*M_PI) *sK* tau) * exp(-1./2 * pow(x/sK,2) + d*d/c);
      double Const = 1./(sqrt(2*M_PI) *sK* tau) * exp(-x/tau + 1./2 *pow(sK/tau,2));

      return 1./(2*tau) * TMath::Erfc(1./sqrt(2) * (-x/sK + sK/tau)) *  exp(-x/tau + 1./2 *pow(sK/tau,2));
      */

      // safety term
      double A = 1. / sqrt(2) * (-x / sK + sK / tau);
      double B = -x / tau + 1. / 2 * pow(sK / tau, 2);
      double res = 0;
      if (B > 700 || A > 20) {
        res = 1. / (2 * tau) * 1. / sqrt(M_PI) * exp(-A * A + B) * (1 / A - 1 / 2. / pow(A, 3) + 3. / 4 / pow(A, 5));
      } else {
        res = 1. / (2 * tau) * TMath::Erfc(A) * exp(B);
      }
      assert(isfinite(res));
      return res;
    }

    double expDoubleSided(const double* par)
    {
      double x = par[0];
      double mean = par[1];
      double f      = par[2];
      double tauR   = par[3];
      double tauL   = par[4];


      if (x >= mean)
        return f * tauR * exp(-abs(x - mean) / tauR)  ;
      else
        return (1 - f) * tauL * exp(-abs(x - mean) / tauL);
    }


    double gausExpConv(double mean, double sigma, double bMean, double bDelta, double tauL, double tauR, double sigmaK, double x)
    {
      double bL = bMean - bDelta;
      double bR = bMean + bDelta;

      double xL = bL * sigma + mean;
      double xR = bR * sigma + mean;

      double iGaus  = sqrt(2 * M_PI) * sigma * convGausGaus(sigmaK, sigma, xL, xR, mean, x);
      double iRight = exp(-1. / 2 * pow(bR, 2)) * tauR * convExpGaus(sigmaK, tauR, xR, x);
      double iLeft  = exp(-1. / 2 * pow(bL, 2)) * tauL * convExpGaus(sigmaK, tauL, -xL, -x);

      return (iGaus + iLeft + iRight);
    }

    double gausExpConvRoot(const double* par)
    {
      double x = par[0];
      double mean = par[1];
      double sigma = par[2]; //sigma of Gauss in Crystal Ball
      double bMean = par[3];
      double bDelta = par[4];
      double tauL = par[5]; //exp slopes of tails
      double tauR = par[6];
      double sigmaK = par[7]; //sigma of the Gaussian Kernel
      double sigmaA = par[8]; //sigma of the added Gaussian
      double fA     = par[9]; //sigma of the added Gaussian

      //added gaussian
      double G = 1. / (sqrt(2 * M_PI) * sigmaA) * exp(-1. / 2 * pow((x - mean) / sigmaA, 2));
      return (1 - fA) * gausExpConv(mean, sigma, bMean, bDelta, tauL, tauR, sigmaK, x) + fA * G;
    }

    double gausExpPowConvRoot(const double* par)
    {
      double x = par[0];
      double mean = par[1];
      double sigma = par[2]; //sigma of Gauss in Crystal Ball
      double bMean = par[3];
      double bDelta = par[4];
      double tauL = par[5]; //exp slopes of tails
      double tauR = par[6];
      double sigmaK = par[7]; //sigma of the Gaussian Kernel
      double sigmaA = par[8]; //sigma of the added Gaussian
      double fA     = par[9]; //sigma of the added Gaussian



      double eCMS  = par[10];
      double slope = par[11];
      double K = par[12];

      double eps   = 0.10;

      double sum = 0;
      double step = eps;

      int N = 800 * 1. / eps;

      for (int i = 0; i < N; ++i) {
        //for(double t = eCMS - eps;  t > eCMS - eps; t -= step) {

        double t = eCMS - i * step;

        double y = x - t;
        double G = 1. / (sqrt(2 * M_PI) * sigmaA) * exp(-1. / 2 * pow((y - mean) / sigmaA, 2));
        double Core = (1 - fA) * gausExpConv(mean, sigma, bMean, bDelta, tauL, tauR, sigmaK, y) + fA * G;

        double C = (i == 0 || i == N - 1) ? 0.5 : 1;

        //f(t) * K(x - t)
        double Kernel;
        if (i == 0)
          Kernel = K * pow(step, -slope);
        else
          Kernel = pow(eCMS - t, -slope);


        sum += Kernel * Core * step * C;
      }

      return sum;
    }







    void plotTest()
    {
      double mean = 0;
      double bMean = 0;
      double bDelta = 5;
      double tauL = 30;
      double tauR = 30;
      double sigma = 10;
      double sigmaK = 40;

      TGraph* gr = new TGraph;
      for (double x = -300; x < 300; x += 1) {

        double v = gausExpConv(mean, sigma, bMean, bDelta, tauL, tauR, sigmaK, x);

        gr->SetPoint(gr->GetN(), x, v);
      }

      gr->Draw();
    }


    double gausExp(const double* par)
    {
      double x = par[0];
      double mean = par[1];
      double sigma = par[2];
      double bMean = par[3];
      double bDelta = par[4];
      double tauL = par[5];
      double tauR = par[6];

      double bL = bMean - bDelta;
      double bR = bMean + bDelta;


      double r = (x - mean) / sigma;
      if (bL <= r && r <= bR) {
        return exp(-1. / 2 * pow(r, 2));
      } else if (r < bL) {
        double bp = exp(-1. / 2 * pow(bL, 2));
        double xb = mean + bL * sigma;
        return exp((x - xb) / tauL) * bp;
      } else {
        double bp = exp(-1. / 2 * pow(bR, 2));
        double xb = mean + bR * sigma;
        return exp(-(x - xb) / tauR) * bp;
      }
    }


    double gausExpSum(const double* par)
    {
      double x = par[0];
      double mean = par[1];
      //double sigma = par[2];
      double tauL = par[3];
      double tauR = par[4];
      double f    = par[5];
      double fG   = par[6];


      if (x < mean) {
        return (1 - fG) * f * exp(-abs(x - mean) / tauL) / tauL;
      } else {
        return (1 - fG) * (1 - f) * exp(-abs(x - mean) / tauR) / tauR;
      }

    }




    /** The integrator aims to evaluate PDGgen conv ResFunction */
    struct Integrator {

      double mean = 4;    ///< mean position of the resolution function, i.e. (Gaus+Exp tails) conv Gaus
      double sigma  = 30; ///< sigma of the resolution function
      double sigmaK = 30; ///< sigma of the gaus in the convolution
      double bMean = 0;    ///< (bRight + bLeft)/2 where bLeft, bRight are the transition points between gaus and exp (in sigma)
      double bDelta = 2.6; ///< (bRight - bLeft)/2 where bLeft, bRight are the transition points between gaus and exp (in sigma)
      double tauL = 60;    ///< 1/slope of the left exponential tail
      double tauR = 60;    ///< 1/slope of the right exponential tail

      double sigmaE = 30;  ///< sigma of the external gaus
      double frac = 0.1;   ///< fraction of events in the external gaus

      double m0   = 10500; ///< invariant mass of the collisions
      double eps  = 0.01;  ///< cut-off term for the power-spectrum caused by the ISR (in GeV)
      double slope = 0.95; ///< power in the power-like spectrum from the ISR

      double x    = 10400; ///< the resulting PDF is function of this variable the actual mass

      double C = 16;       ///< the coeficient between part bellow eps and above eps cut-off

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
                double X)
      {
        mean   = Mean;
        sigma  = Sigma;
        sigmaK = SigmaK;
        bMean  = BMean;
        bDelta = BDelta;
        tauL   = Tau;
        tauR   = Tau;
        sigmaE = SigmaE;
        frac   = Frac;

        m0 = M0;
        eps = Eps;
        C  = CC;
        slope = Slope;

        x      = X;
      }

      /** evaluate the PDF integrand for given t - the integration variable */
      double eval(double t)
      {
        double CoreC = gausExpConv(mean, sigma, bMean, bDelta, tauL, tauR, sigmaK, x + t - m0);
        double CoreE = 1. / (sqrt(2 * M_PI) * sigmaE) * exp(-1. / 2 * pow((x + t - m0 - mean) / sigmaE, 2));
        double Core = (1 - frac) * CoreC + frac * CoreE;

        assert(t >= 0);

        double K = 0;
        if (t >= eps)
          K = pow(t, -slope);
        else if (t >= 0)
          K = (1 + (C - 1) * (eps - t) / eps) * pow(eps, -slope);
        else
          K = 0;

        //double K = (+t) >= eps ? pow( + t, -slope) : 0;
        return Core * K;
      }

      /** Simple integration of the PDF based on the Trapezoidal rule */
      double integralTrap(double a, double b)
      {
        const int N = 14500000;
        double step = (b - a) / N;
        double s = 0;
        for (int i = 0; i <= N; ++i) {
          double K = (i == 0 || i == N) ? 0.5 : 1;
          double t = a + i * step;
          s += eval(t) * step * K;
        }


        return s;
      }

      /** Integration of the PDF which avoids steps and uses variable transformation (Trapezoidal rule as back-end) */
      double integralTrapImp(double Eps, double a)
      {
        double tMin = slope * tauL;

        //only one function type
        if (x - m0 >= 0) {

          double r1 = ROOT::Math::inc_gamma_c(1 - slope,   a   / tauL);
          double r2 = ROOT::Math::inc_gamma_c(1 - slope,   Eps / tauL);


          const int N = 15000;
          double step = (r2 - r1) / N;
          double s = 0;
          for (int i = 0; i <= N; ++i) {
            double r = r1 + step * i;
            double t = tauL * ROOT::Math::gamma_quantile_c(r, 1 - slope, 1);
            double K = (i == 0 || i == N) ? 0.5 : 1;

            double est = pow(t, -slope) * exp(- (x - m0 + t) / tauL);

            s +=  eval(t) / est *  step * K;
          }

          s *= exp((m0 - x) / tauL) * pow(tauL, -slope + 1) * ROOT::Math::tgamma(1 - slope);

          return s;

        }

        else if (x - m0 >= -tMin) {

          //integrate from m0 - x  to a
          double s1 = 0;

          {
            double r1 = ROOT::Math::inc_gamma_c(1 - slope,   a   / tauL);
            double r2 = ROOT::Math::inc_gamma_c(1 - slope, (m0 - x) / tauL);


            const int N = 150000;
            double step = (r2 - r1) / N;
            for (int i = 0; i <= N; ++i) {
              double r = r1 + step * i;
              double t = tauL * ROOT::Math::gamma_quantile_c(r, 1 - slope, 1);
              double K = (i == 0 || i == N) ? 0.5 : 1;

              double est = pow(t, -slope) * exp(- (x - m0 + t) / tauL);

              s1 +=  eval(t) / est *  step * K;
            }

            s1 *= exp((m0 - x) / tauL) * pow(tauL, -slope + 1) * ROOT::Math::tgamma(1 - slope);
          }


          // integrate from eps to (m0 - x)
          double s2 = 0;

          {
            double r1 = pow(Eps, -slope + 1) / (1 - slope);
            double r2 = pow(m0 - x, -slope + 1) / (1 - slope);


            const int N = 150000;
            double step = (r2 - r1) / N;
            for (int i = 0; i <= N; ++i) {
              double r = r1 + step * i;
              double t = pow(r * (1 - slope), 1. / (1 - slope));
              double K = (i == 0 || i == N) ? 0.5 : 1;

              double est = pow(t, -slope);

              s2 +=  eval(t) / est *  step * K;
            }

          }

          return (s1  + s2);

        } else {

          //integrate from m0 - x  to a
          double s1 = 0;

          {
            double r1 = ROOT::Math::inc_gamma_c(1 - slope,   a   / tauL);
            double r2 = ROOT::Math::inc_gamma_c(1 - slope, (m0 - x) / tauL);


            const int N = 150000;
            double step = (r2 - r1) / N;
            for (int i = 0; i <= N; ++i) {
              double r = r1 + step * i;
              double t = tauL * ROOT::Math::gamma_quantile_c(r, 1 - slope, 1);
              double K = (i == 0 || i == N) ? 0.5 : 1;

              double est = pow(t, -slope) * exp(- (x - m0 + t) / tauL);

              s1 +=  eval(t) / est *  step * K;
            }

            s1 *= exp((m0 - x) / tauL) * pow(tauL, -slope + 1) * ROOT::Math::tgamma(1 - slope);
          }


          // integrate from eps to tMin
          double s2 = 0;

          {
            double r1 = pow(Eps, -slope + 1) / (1 - slope);
            double r2 = pow(tMin, -slope + 1) / (1 - slope);


            const int N = 150000;
            double step = (r2 - r1) / N;
            for (int i = 0; i <= N; ++i) {
              double r = r1 + step * i;
              double t = pow(r * (1 - slope), 1. / (1 - slope));
              double K = (i == 0 || i == N) ? 0.5 : 1;

              double est = pow(t, -slope);

              s2 +=  eval(t) / est *  step * K;
            }

          }

          //integrate from tMin to m0 - x
          double s3 = 0;

          {
            double r1 = exp(tMin / tauR) * tauR;
            double r2 = exp((m0 - x) / tauR) * tauR;


            const int N = 150000;
            double step = (r2 - r1) / N;
            for (int i = 0; i <= N; ++i) {
              double r = r1 + step * i;
              double t = log(r / tauR) * tauR;
              double K = (i == 0 || i == N) ? 0.5 : 1;

              double est = exp(t / tauR);

              s3 +=  eval(t) / est *  step * K;
            }


          }

          return (s1 + s2 + s3);

        }

        return 0;

      }


      /** Integration of the PDF which avoids steps and uses variable transformation (Gauss-Konrod rule as back-end) */
      double integralKronrod(double a)
      {

        double s0 = integrate([&](double t) {
          return eval(t);
        }   , 0, eps);


        double tMin = slope * tauL;

        //only one function type
        if (x - m0 >= 0) {

          double r1 = ROOT::Math::inc_gamma_c(1 - slope,   a   / tauL);
          double r2 = ROOT::Math::inc_gamma_c(1 - slope,   eps / tauL);

          double s = integrate([&](double r) {

            double t = tauL * ROOT::Math::gamma_quantile_c(r, 1 - slope, 1);
            double est = pow(t, -slope) * exp(- (x - m0 + t) / tauL);
            return eval(t) / est;


          }   , r1, r2);

          s *= exp((m0 - x) / tauL) * pow(tauL, -slope + 1) * ROOT::Math::tgamma(1 - slope);

          return (s0 + s);

        }

        else if (x - m0 >= -tMin) {

          //integrate from m0 - x  to a
          double s1 = 0;

          {
            double r1 = ROOT::Math::inc_gamma_c(1 - slope,   a   / tauL);
            double r2 = ROOT::Math::inc_gamma_c(1 - slope, (m0 - x) / tauL);


            s1 = integrate([&](double r) {

              double t = tauL * ROOT::Math::gamma_quantile_c(r, 1 - slope, 1);
              double est = pow(t, -slope) * exp(- (x - m0 + t) / tauL);
              return eval(t) / est;


            }   , r1, r2);

            s1 *= exp((m0 - x) / tauL) * pow(tauL, -slope + 1) * ROOT::Math::tgamma(1 - slope);

          }


          // integrate from eps to (m0 - x)
          double s2 = 0;

          {
            double r1 = pow(eps, -slope + 1) / (1 - slope);
            double r2 = pow(m0 - x, -slope + 1) / (1 - slope);


            s2 = integrate([&](double r) {
              double t = pow(r * (1 - slope), 1. / (1 - slope));
              double est = pow(t, -slope);
              return eval(t) / est;
            }   , r1, r2);
          }

          return (s0 + s1  + s2);

        } else {

          //integrate from m0 - x  to a
          double s1 = 0;

          {
            double r1 = ROOT::Math::inc_gamma_c(1 - slope,   a   / tauL);
            double r2 = ROOT::Math::inc_gamma_c(1 - slope, (m0 - x) / tauL);

            s1 = integrate([&](double r) {

              double t = tauL * ROOT::Math::gamma_quantile_c(r, 1 - slope, 1);
              double est = pow(t, -slope) * exp(- (x - m0 + t) / tauL);
              return eval(t) / est;


            }   , r1, r2);

            s1 *= exp((m0 - x) / tauL) * pow(tauL, -slope + 1) * ROOT::Math::tgamma(1 - slope);

          }


          // integrate from eps to tMin
          double s2 = 0;

          {
            double r1 = pow(eps, -slope + 1) / (1 - slope);
            double r2 = pow(tMin, -slope + 1) / (1 - slope);

            s2 = integrate([&](double r) {
              double t = pow(r * (1 - slope), 1. / (1 - slope));
              double est = pow(t, -slope);
              return eval(t) / est;
            }   , r1, r2);

          }

          //integrate from tMin to m0 - x
          double s3 = 0;

          {
            double r1 = exp(tMin / tauR) * tauR;
            double r2 = exp((m0 - x) / tauR) * tauR;


            s3 = integrate([&](double r) {
              double t = log(r / tauR) * tauR;
              double est = exp(t / tauR);
              return eval(t) / est;
            }   , r1, r2);

          }

          return (s0 + s1 + s2 + s3);


        }

        return 0;

      }


    };


    void plotInt()
    {
      double mean = 4;
      double sigma  = 30;
      double sigmaK = 30;
      double bMean = 0;
      double bDelta = 2.6;
      double tau = 60;

      double x = 10300;

      TGraph* gr    = new TGraph;
      TGraph* grE   = new TGraph;
      TGraph* grRat = new TGraph;
      TGraph* grR   = new TGraph;

      double m0 = 10500;
      double slope = 0.95;
      double eps = 0.01;

      double frac = 0.2;
      double sigmaE = 30;

      for (double t = eps; t < 5000; t += 1.00) {
        double Core = gausExpConv(mean, sigma, bMean, bDelta, tau, tau, sigmaK, x + t - m0);
        double K = (+t) >= eps ? pow(+ t, -slope) : 0;

        double fun = Core * K;
        gr->SetPoint(gr->GetN(), t, fun);

        //double s = ROOT::Math::inc_gamma_c(-0.95 + 1,   t/tau);
        double s = exp(-t / tau);

        double funE = exp(-abs(x - m0 + t) / tau) * 1 / t;
        grE->SetPoint(grE->GetN(), t, funE);
        if (funE > 0) {
          grRat->SetPoint(grE->GetN(), t, fun / funE);
          grR->SetPoint(grR->GetN(), s, fun / funE);
        }
      }


      Integrator integrator;

      TGraph* grM   = new TGraph;

      double C = 16;

      for (double xNow = 10000; xNow <= 11000; xNow += 0.1) {

        integrator.init(mean,
                        sigma,
                        sigmaK,
                        bMean,
                        bDelta,
                        tau,
                        sigmaE,
                        frac,
                        m0,
                        eps,
                        C,
                        slope,
                        xNow);

        //cout << "Trap integration method " << integrator.integralTrap(eps, 2000, false)  + integrator.integralTrap(0, eps, true) << endl;
        //cout << "Konrod integration method " << integrator.integralKronrod(2000) << endl;

        grM->SetPoint(grM->GetN(), xNow, integrator.integralKronrod(2000));
      }

      grM->Draw();



      //grR->Draw();

      //grRat->Draw();

      //gr->Draw();
      //grE->Draw("same");

    }



    double mainFunction(double xx, Pars par)
    {
      Integrator fun;

      fun.x     = xx;
      fun.mean  = par.at("mean");
      fun.sigma = par.at("sigma"); //sigma of Gauss in Crystal Ball
      fun.bMean = par.at("bMean");
      fun.bDelta = par.at("bDelta");
      fun.tauL  = par.at("tau"); //exp slopes of tails
      fun.tauR  = par.at("tau");
      fun.sigmaK = par.at("sigma"); //sigma of the Gaussian Kernel
      fun.sigmaE = par.at("sigma"); //sigma of the added Gaussian
      fun.frac  = par.at("frac"); //frac of the added Gaussian



      fun.m0    = par.at("m0");
      fun.slope = par.at("slope");
      fun.C     = par.at("C");

      fun.eps = 0.1;

      return fun.integralKronrod(2000);
    }


    vector<double> readEvents(const vector<Event>& evts, double pidCut, double a, double b)
    {

      vector<double> vMass;
      for (const auto& ev : evts) {

        //Keep only muons
        if (ev.mu0.pid < pidCut || ev.mu1.pid < pidCut) {
          continue;
        }

        double m = 1e3 * ev.m;
        if (a < m && m < b)
          vMass.push_back(m);
      }

      return vMass;
    }


    /** run the collision invariant mass calibration */
    vector<double>  getInvMassPars(const vector<Event>& evts, int bootStrap = 0)
    {
      double mMin = 10.2e3, mMax = 10.8e3;

      vector<double> dataNow = readEvents(evts, 0.9/*PIDcut*/, mMin, mMax);

      // do bootStrap
      vector<double> data;
      if (bootStrap) gRandom->SetSeed(bootStrap);
      for (auto d : dataNow) {
        int nP = bootStrap ? gRandom->Poisson(1) : 1;
        for (int i = 0; i < nP; ++i)
          data.push_back(d);
      }


      ChebFitter fitter;
      fitter.setDataAndFunction(mainFunction, data);
      fitter.init(256 + 1, mMin, mMax);


      Pars pars = {
        {"C" , 15        },
        {"bDelta" , 1.60307        },
        {"bMean" , 0        },
        {"frac" , 0.998051        },
        {"m0" , 10570.2        },
        {"mean" , 4.13917        },
        {"sigma" , 37.0859        },
        {"slope" , 0.876812        },
        {"tau" , 99.4225}
      };




      Limits limits = {
        {"mean",   make_pair(0, 0)},
        {"sigma",  make_pair(10, 120)},
        {"bMean",  make_pair(0, 0)},
        {"bDelta", make_pair(0.01, 10.)},
        {"tau",    make_pair(20, 250)},
        // {"sigmaK",  3.04175e+01, 10, 120},
        // {"sigmaE",  3.04175e+01, 10, 120},
        {"frac",   make_pair(0.00, 1.0)},

        {"m0",    make_pair(10500, 10700)},
        {"slope", make_pair(0.3, 0.999)},
        {"C",     make_pair(0, 0)}
      };





      //gRandom->SetSeed(10);
      //for(auto &p : pars) {
      //   if(p.vMin == p.vMax) continue;
      //   p.v = gRandom->Uniform(p.vMin, p.vMax);
      //}


      //auto res = fitter.getMinimum(pars);
      //cout << "Pars size " << pars.size() << endl;
      Pars resP;
      MatrixXd resM;
      tie(resP, resM) = fitter.fitData(pars, limits, true/*useCheb*/);

      /*
      TH1D *h = new TH1D("h", "", 100, mMin, mMax);
      for(auto v : data) h->Fill(v);
      h->Scale(1./h->Integral(), "width");

      //plot result
      TGraph *grOrg = new TGraph();
      TGraph *gr = new TGraph();
      for(double x = mMin; x <= mMax; x += 0.1) {
          double v = fitter.getFunctionFast(resP, x);
          double vOrg = mainFunction(x, resP) /74;
          //cout << x <<" " << v << endl;
          gr->SetPoint(gr->GetN(), x, v);
          grOrg->SetPoint(gr->GetN(), x, vOrg);
      }
      TCanvas *c = new TCanvas("can", "");
      gr->Draw("al");
      //grOrg->SetLineColor(kRed);
      //grOrg->Draw("l same");
      h->Draw("same");
      c->SaveAs("plot.pdf");

      */

      int ind = distance(resP.begin(), resP.find("m0"));
      double mass = resP.at("m0");
      double err  = sqrt(resM(ind, ind));

      return { mass, err, 0}; //spread is set to zero
    }




    // Returns tuple with the invariant mass parameters (cmsEnergy in GeV)
    tuple<vector<VectorXd>, vector<MatrixXd>, MatrixXd>  runMuMuInvariantMassAnalysis(vector<Event> evts,
        const vector<double>& splitPoints)
    {
      int n = splitPoints.size() + 1;

      vector<VectorXd>     invMassVec(n);
      vector<MatrixXd>  invMassVecUnc(n);
      MatrixXd          invMassVecSpred;

      std::ofstream mumuTextOut("mumuEcalib.txt", ios::app);

      for (int iDiv = 0; iDiv < n; ++iDiv) {


        invMassVec[iDiv].resize(1);       //1D vector for center of the 1D Gauss
        invMassVecUnc[iDiv].resize(1, 1); //1x1 matrix covariance mat of the center
        invMassVecSpred.resize(1, 1);  //1x1 matrix for spread of the 1D Gauss

        vector<Event> evtsNow;
        for (auto ev :  evts) {
          double tMin = (iDiv != 0)   ? splitPoints[iDiv - 1] : -1e40;
          double tMax = (iDiv != n - 1) ? splitPoints[iDiv]   :  1e40;
          if (tMin <= ev.t && ev.t < tMax)
            evtsNow.push_back(ev);

        }

        const int nRep = 16;

        vector<double> vals, errs;
        for (int rep = 0; rep < 200; ++rep) {
          double errEst = 50. / sqrt(evtsNow.size());
          auto res =  getInvMassPars(evtsNow, rep);
          if (errEst < res[1] && res[1] < 4 * errEst) {
            vals.push_back(res[0]);
            errs.push_back(res[1]);
            if (vals.size() >= nRep) break;
          }
        }

        B2ASSERT("Consistency with number of replicas", vals.size() == nRep);

        double meanMass    = accumulate(vals.begin(), vals.end(), 0.) / vals.size();
        double meanMassUnc = accumulate(errs.begin(), errs.end(), 0.) / errs.size();

        double sum2 = 0;
        for (auto v : vals)
          sum2 += pow(v - meanMass, 2);

        mumuTextOut << n << " " << iDiv << " " << setprecision(14) << evtsNow.front().t << " " << evtsNow.back().t << " : " << meanMass <<
                    " +- " << meanMassUnc << " " << sqrt(sum2 / (vals.size() - 1)) <<   endl;

        // Convert to GeV
        invMassVec[iDiv](0) = meanMass / 1e3;
        invMassVecUnc[iDiv](0, 0) = meanMassUnc / 1e3;
        invMassVecSpred(0, 0) = 0;
      }

      mumuTextOut.close();

      return make_tuple(invMassVec, invMassVecUnc, invMassVecSpred);
    }

  }
}
