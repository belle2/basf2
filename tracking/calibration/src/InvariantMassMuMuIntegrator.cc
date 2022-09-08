/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



#include <tracking/calibration/InvariantMassMuMuIntegrator.h>
#include <tracking/calibration/InvariantMassMuMuStandAlone.h>

#include <cassert>
#include <cmath>

#include <Math/SpecFuncMathCore.h>
#include <Math/DistFunc.h>

namespace Belle2::InvariantMassMuMuCalib {

  void InvariantMassMuMuIntegrator::init(double Mean,
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
    m_mean   = Mean;
    m_sigma  = Sigma;
    m_sigmaK = SigmaK;
    m_bMean  = BMean;
    m_bDelta = BDelta;
    m_tauL   = Tau;
    m_tauR   = Tau;
    m_sigmaE = SigmaE;
    m_frac   = Frac;

    m_m0 = M0;
    m_eps = Eps;
    m_C  = CC;
    m_slope = Slope;

    m_x      = X;
  }




  /** evaluate the PDF integrand for given t - the integration variable */
  double  InvariantMassMuMuIntegrator::eval(double t)
  {
    double CoreC = gausExpConv(m_mean, m_sigma, m_bMean, m_bDelta, m_tauL, m_tauR, m_sigmaK, m_x + t - m_m0);
    double CoreE = 1. / (sqrt(2 * M_PI) * m_sigmaE) * exp(-1. / 2 * pow((m_x + t - m_m0 - m_mean) / m_sigmaE, 2));
    double Core = (1 - m_frac) * CoreC + m_frac * CoreE;

    assert(t >= 0);

    double K = 0;
    if (t >= m_eps)
      K = pow(t, -m_slope);
    else if (t >= 0)
      K = (1 + (m_C - 1) * (m_eps - t) / m_eps) * pow(m_eps, -m_slope);
    else
      K = 0;

    return Core * K;
  }

  // Simple integration of the PDF based on the Trapezoidal rule
  double InvariantMassMuMuIntegrator::integralTrap(double a, double b)
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

  // Integration of the PDF which avoids steps and uses variable transformation (Trapezoidal rule as back-end)
  double InvariantMassMuMuIntegrator::integralTrapImp(double Eps, double a)
  {
    double tMin = m_slope * m_tauL;

    //only one function type
    if (m_x - m_m0 >= 0) {

      double r1 = ROOT::Math::inc_gamma_c(1 - m_slope,   a   / m_tauL);
      double r2 = ROOT::Math::inc_gamma_c(1 - m_slope,   Eps / m_tauL);


      const int N = 15000;
      double step = (r2 - r1) / N;
      double s = 0;
      for (int i = 0; i <= N; ++i) {
        double r = r1 + step * i;
        double t = m_tauL * ROOT::Math::gamma_quantile_c(r, 1 - m_slope, 1);
        double K = (i == 0 || i == N) ? 0.5 : 1;

        double est = pow(t, -m_slope) * exp(- (m_x - m_m0 + t) / m_tauL);

        s +=  eval(t) / est *  step * K;
      }

      s *= exp((m_m0 - m_x) / m_tauL) * pow(m_tauL, -m_slope + 1) * ROOT::Math::tgamma(1 - m_slope);

      return s;

    }

    else if (m_x - m_m0 >= -tMin) {

      //integrate from m_m0 - m_x  to a
      double s1 = 0;

      {
        double r1 = ROOT::Math::inc_gamma_c(1 - m_slope,   a   / m_tauL);
        double r2 = ROOT::Math::inc_gamma_c(1 - m_slope, (m_m0 - m_x) / m_tauL);


        const int N = 150000;
        double step = (r2 - r1) / N;
        for (int i = 0; i <= N; ++i) {
          double r = r1 + step * i;
          double t = m_tauL * ROOT::Math::gamma_quantile_c(r, 1 - m_slope, 1);
          double K = (i == 0 || i == N) ? 0.5 : 1;

          double est = pow(t, -m_slope) * exp(- (m_x - m_m0 + t) / m_tauL);

          s1 +=  eval(t) / est *  step * K;
        }

        s1 *= exp((m_m0 - m_x) / m_tauL) * pow(m_tauL, -m_slope + 1) * ROOT::Math::tgamma(1 - m_slope);
      }


      // integrate from eps to (m_m0 - m_x)
      double s2 = 0;

      {
        double r1 = pow(Eps, -m_slope + 1) / (1 - m_slope);
        double r2 = pow(m_m0 - m_x, -m_slope + 1) / (1 - m_slope);


        const int N = 150000;
        double step = (r2 - r1) / N;
        for (int i = 0; i <= N; ++i) {
          double r = r1 + step * i;
          double t = pow(r * (1 - m_slope), 1. / (1 - m_slope));
          double K = (i == 0 || i == N) ? 0.5 : 1;

          double est = pow(t, -m_slope);

          s2 +=  eval(t) / est *  step * K;
        }

      }

      return (s1  + s2);

    } else {

      //integrate from m_m0 - m_x  to a
      double s1 = 0;

      {
        double r1 = ROOT::Math::inc_gamma_c(1 - m_slope,   a   / m_tauL);
        double r2 = ROOT::Math::inc_gamma_c(1 - m_slope, (m_m0 - m_x) / m_tauL);


        const int N = 150000;
        double step = (r2 - r1) / N;
        for (int i = 0; i <= N; ++i) {
          double r = r1 + step * i;
          double t = m_tauL * ROOT::Math::gamma_quantile_c(r, 1 - m_slope, 1);
          double K = (i == 0 || i == N) ? 0.5 : 1;

          double est = pow(t, -m_slope) * exp(- (m_x - m_m0 + t) / m_tauL);

          s1 +=  eval(t) / est *  step * K;
        }

        s1 *= exp((m_m0 - m_x) / m_tauL) * pow(m_tauL, -m_slope + 1) * ROOT::Math::tgamma(1 - m_slope);
      }


      // integrate from eps to tMin
      double s2 = 0;

      {
        double r1 = pow(Eps, -m_slope + 1) / (1 - m_slope);
        double r2 = pow(tMin, -m_slope + 1) / (1 - m_slope);


        const int N = 150000;
        double step = (r2 - r1) / N;
        for (int i = 0; i <= N; ++i) {
          double r = r1 + step * i;
          double t = pow(r * (1 - m_slope), 1. / (1 - m_slope));
          double K = (i == 0 || i == N) ? 0.5 : 1;

          double est = pow(t, -m_slope);

          s2 +=  eval(t) / est *  step * K;
        }

      }

      //integrate from tMin to m_m0 - m_x
      double s3 = 0;

      {
        double r1 = exp(tMin / m_tauR) * m_tauR;
        double r2 = exp((m_m0 - m_x) / m_tauR) * m_tauR;


        const int N = 150000;
        double step = (r2 - r1) / N;
        for (int i = 0; i <= N; ++i) {
          double r = r1 + step * i;
          double t = log(r / m_tauR) * m_tauR;
          double K = (i == 0 || i == N) ? 0.5 : 1;

          double est = exp(t / m_tauR);

          s3 +=  eval(t) / est *  step * K;
        }


      }

      return (s1 + s2 + s3);

    }

    return 0;

  }


  // Integration of the PDF which avoids steps and uses variable transformation (Gauss-Konrod rule as back-end)
  double InvariantMassMuMuIntegrator::integralKronrod(double a)
  {

    double s0 = integrate([&](double t) {
      return eval(t);
    }, 0, m_eps);


    double tMin = m_slope * m_tauL;

    //only one function type
    if (m_x - m_m0 >= 0) {

      assert(m_eps         <= a);

      double r1 = ROOT::Math::inc_gamma_c(1 - m_slope,   a   / m_tauL);
      double r2 = ROOT::Math::inc_gamma_c(1 - m_slope,   m_eps / m_tauL);

      double s = integrate([&](double r) {

        double t = m_tauL * ROOT::Math::gamma_quantile_c(r, 1 - m_slope, 1);
        double est = pow(t, -m_slope) * exp(- (m_x - m_m0 + t) / m_tauL);
        return eval(t) / est;


      }, r1, r2);

      s *= exp((m_m0 - m_x) / m_tauL) * pow(m_tauL, -m_slope + 1) * ROOT::Math::tgamma(1 - m_slope);

      return (s0 + s);

    }

    //only one function type
    else if (m_x - m_m0 >= -m_eps) {

      assert(0 <= m_m0 - m_x);
      assert(m_m0 - m_x <= m_eps);
      assert(m_eps <= a);


      double s01 = integrate([&](double t) {
        return eval(t);
      }, 0, m_m0 - m_x);


      double s02 = integrate([&](double t) {
        return eval(t);
      }, m_m0 - m_x, m_eps);


      double r1 = ROOT::Math::inc_gamma_c(1 - m_slope,   a   / m_tauL);
      double r2 = ROOT::Math::inc_gamma_c(1 - m_slope,   m_eps / m_tauL);

      double s = integrate([&](double r) {

        double t = m_tauL * ROOT::Math::gamma_quantile_c(r, 1 - m_slope, 1);
        double est = pow(t, -m_slope) * exp(- (m_x - m_m0 + t) / m_tauL);
        return eval(t) / est;


      }, r1, r2);

      s *= exp((m_m0 - m_x) / m_tauL) * pow(m_tauL, -m_slope + 1) * ROOT::Math::tgamma(1 - m_slope);

      return (s01 + s02 + s);

    }


    //two function types
    else if (m_x - m_m0 >= -tMin) {

      assert(m_eps       <= m_m0 - m_x);
      assert(m_m0 - m_x <= a);

      //integrate from m_m0 - m_x  to a
      double s1 = 0;

      {
        double r1 = ROOT::Math::inc_gamma_c(1 - m_slope,   a   / m_tauL);
        double r2 = ROOT::Math::inc_gamma_c(1 - m_slope, (m_m0 - m_x) / m_tauL);


        s1 = integrate([&](double r) {

          double t = m_tauL * ROOT::Math::gamma_quantile_c(r, 1 - m_slope, 1);
          double est = pow(t, -m_slope) * exp(- (m_x - m_m0 + t) / m_tauL);
          return eval(t) / est;


        }, r1, r2);

        s1 *= exp((m_m0 - m_x) / m_tauL) * pow(m_tauL, -m_slope + 1) * ROOT::Math::tgamma(1 - m_slope);

      }


      // integrate from eps to (m_m0 - m_x)
      double s2 = 0;

      {
        double r1 = pow(m_eps, -m_slope + 1) / (1 - m_slope);
        double r2 = pow(m_m0 - m_x, -m_slope + 1) / (1 - m_slope);



        s2 = integrate([&](double r) {
          double t = pow(r * (1 - m_slope), 1. / (1 - m_slope));
          double est = pow(t, -m_slope);
          return eval(t) / est;
        }, r1, r2);
      }

      return (s0 + s1  + s2);

    } else {

      assert(m_eps       <= tMin);
      assert(tMin      <= m_m0 - m_x);
      assert(m_m0 - m_x <= a);

      //integrate from m_m0 - m_x  to a
      double s1 = 0;

      {
        double r1 = ROOT::Math::inc_gamma_c(1 - m_slope,   a   / m_tauL);
        double r2 = ROOT::Math::inc_gamma_c(1 - m_slope, (m_m0 - m_x) / m_tauL);


        s1 = integrate([&](double r) {

          double t = m_tauL * ROOT::Math::gamma_quantile_c(r, 1 - m_slope, 1);
          double est = pow(t, -m_slope) * exp(- (m_x - m_m0 + t) / m_tauL);
          return eval(t) / est;


        }, r1, r2);

        s1 *= exp((m_m0 - m_x) / m_tauL) * pow(m_tauL, -m_slope + 1) * ROOT::Math::tgamma(1 - m_slope);

      }


      // integrate from eps to tMin
      double s2 = 0;

      {
        double r1 = pow(m_eps, -m_slope + 1) / (1 - m_slope);
        double r2 = pow(tMin, -m_slope + 1) / (1 - m_slope);

        s2 = integrate([&](double r) {
          double t = pow(r * (1 - m_slope), 1. / (1 - m_slope));
          double est = pow(t, -m_slope);
          return eval(t) / est;
        }, r1, r2);

      }

      //integrate from tMin to m_m0 - m_x
      double s3 = 0;

      {
        double r1 = exp(tMin / m_tauR) * m_tauR;
        double r2 = exp((m_m0 - m_x) / m_tauR) * m_tauR;


        s3 = integrate([&](double r) {
          double t = log(r / m_tauR) * m_tauR;
          double est = exp(t / m_tauR);
          return eval(t) / est;
        }, r1, r2);

      }

      return (s0 + s1 + s2 + s3);


    }

    return 0;

  }

}
