/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Koji Hara                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "EvtGenBase/EvtComplex.hh"
#include "generators/evtgen/EvtBSemiTauonicDecayRateCalculator.h"
#include "generators/evtgen/models/EvtBSemiTauonicHelicityAmplitudeCalculator.h"
#include "framework/logging/Logger.h"
#include <cmath>
#include <boost/format.hpp>

#include "TF1.h"
#include "TF2.h"
#include "Math/WrappedTF1.h"
#include "Math/WrappedMultiTF1.h"
//#include "Math/GSLIntegrator.h" // USABLE ON KEKCC BUT NOT ON BUILDBOT
#include "Math/GaussIntegrator.h"
#include "Math/AdaptiveIntegratorMultiDim.h"
//#include "Math/GSLMCIntegrator.h"

namespace Belle2 {

// dgamma/dw/dcostau
  double EvtBSemiTauonicDecayRateCalculator::dGammadwdcostau(const Belle2::EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD,
                                                             double mtau, int tauhel, int Dhel, double w, double costau)
  {
    EvtComplex temp = BSTD.helAmp(mtau, tauhel, Dhel, w, costau);
    std::complex<double> amp(real(temp), imag(temp));

    return std::norm(amp) * pf(BSTD, mtau, Dhel, w);
  }

// dgamma integrated for costau
  double EvtBSemiTauonicDecayRateCalculator::dGammadw(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD,
                                                      double mtau, int tauhel, int Dhel, double w)
  {
    m_BSTD = &BSTD;
    TF1 f1("f1", this, &EvtBSemiTauonicDecayRateCalculator::EvaluateByCostau, -1, 1, 4,
           "EvtBSemiTauonicDecayRateCalculator", "EvaluateByCostau");
    f1.SetParameter(0, mtau);
    f1.SetParameter(1, tauhel);
    f1.SetParameter(2, Dhel);
    f1.SetParameter(3, w);
    ROOT::Math::WrappedTF1 wf1(f1);
//    ROOT::Math::GSLIntegrator ig;
    ROOT::Math::GaussIntegrator ig;
    ig.SetFunction(wf1);
    return ig.Integral(-1, 1);
  }

// dgamma integrated for w
  double EvtBSemiTauonicDecayRateCalculator::dGammadcostau(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau,
                                                           int tauhel, int Dhel, double costau)
  {
    m_BSTD = &BSTD;
    TF1 f1("f1", this, &EvtBSemiTauonicDecayRateCalculator::EvaluateByW, BSTD.wmin(), BSTD.wmax(mtau, Dhel), 4,
           "EvtBSemiTauonicDecayRateCalculator", "EvaluateByW");
    f1.SetParameter(0, mtau);
    f1.SetParameter(1, tauhel);
    f1.SetParameter(2, Dhel);
    f1.SetParameter(3, costau);
    ROOT::Math::WrappedTF1 wf1(f1);
//    ROOT::Math::GSLIntegrator ig;
    ROOT::Math::GaussIntegrator ig;
    ig.SetFunction(wf1);
    return ig.Integral(BSTD.wmin(), BSTD.wmax(mtau, Dhel));
  }

// gamma integrated for costau and w
  double EvtBSemiTauonicDecayRateCalculator::Gamma(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD,
                                                   double mtau, int tauhel, int Dhel)
  {
    m_BSTD = &BSTD;
    TF2 f1("f1", this, &EvtBSemiTauonicDecayRateCalculator::EvaluateBy2D,
           BSTD.wmin(), BSTD.wmax(mtau, Dhel), -1, 1, 3,
           "EvtBSemiTauonicDecayRateCalculator", "EvaluateBy2D");
    f1.SetParameter(0, mtau);
    f1.SetParameter(1, tauhel);
    f1.SetParameter(2, Dhel);
    ROOT::Math::WrappedMultiTF1 wf1(f1);

    // Create the Integrator
    ROOT::Math::AdaptiveIntegratorMultiDim ig;
    ig.SetFunction(wf1);
    double xmin[] = {BSTD.wmin(), -1};
    double xmax[] = {BSTD.wmax(mtau, Dhel), 1};
    return ig.Integral(xmin, xmax);
  }

//////
  double EvtBSemiTauonicDecayRateCalculator::GammaD(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau)
  {
    double sum(0);
    sum += Gamma(BSTD, mtau, -1, 2);
    sum += Gamma(BSTD, mtau, +1, 2);
    return sum;
  }

  double EvtBSemiTauonicDecayRateCalculator::GammaDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau)
  {
    double sum(0);
    for (int Dhel = -1; Dhel <= 1; Dhel++) {
      sum += Gamma(BSTD, mtau, -1, Dhel);
      sum += Gamma(BSTD, mtau, +1, Dhel);
    }
    return sum;
  }

// SM
  double EvtBSemiTauonicDecayRateCalculator::GammaSMD(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mlep)
  {

    EvtBSemiTauonicHelicityAmplitudeCalculator SM(BSTD);
    SM.setCV1(0);
    SM.setCV2(0);
    SM.setCS1(0);
    SM.setCS2(0);
    SM.setCT(0);
    double sum(0);
    sum += Gamma(SM, mlep, -1, 2);
    sum += Gamma(SM, mlep, +1, 2);
    return sum;
  }

  double EvtBSemiTauonicDecayRateCalculator::GammaSMDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mlep)
  {
    EvtBSemiTauonicHelicityAmplitudeCalculator SM(BSTD);
    SM.setCV1(0);
    SM.setCV2(0);
    SM.setCS1(0);
    SM.setCS2(0);
    SM.setCT(0);
    double sum(0);
    for (int Dhel = -1; Dhel <= 1; Dhel++) {
      sum += Gamma(SM, mlep, -1, Dhel);
      sum += Gamma(SM, mlep, +1, Dhel);
    }
    return sum;
  }

// R(D) and R(Dstar)
  double EvtBSemiTauonicDecayRateCalculator::RGammaD(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau,
                                                     const double mlep)
  {
    double sum(0);
    sum += Gamma(BSTD, mtau, -1, 2);
    sum += Gamma(BSTD, mtau, +1, 2);
    return sum / GammaSMD(BSTD, mlep);
  }

  double EvtBSemiTauonicDecayRateCalculator::RGammaDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau,
                                                         const double mlep)
  {
    double sum(0);
    for (int Dhel = -1; Dhel <= 1; Dhel++) {
      sum += Gamma(BSTD, mtau, -1, Dhel);
      sum += Gamma(BSTD, mtau, +1, Dhel);
    }
    return sum / GammaSMDstar(BSTD, mlep);
  }

// Polarizations
  double EvtBSemiTauonicDecayRateCalculator::PtauD(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau)
  {
    double left(0), right(0);
    left += Gamma(BSTD, mtau, -1, 2);
    right += Gamma(BSTD, mtau, +1, 2);
    return (right - left) / (right + left);
  }
  double EvtBSemiTauonicDecayRateCalculator::PtauDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau)
  {
    double left(0), right(0);
    for (int Dhel = -1; Dhel <= 1; Dhel++) {
      left += Gamma(BSTD, mtau, -1, Dhel);
      right += Gamma(BSTD, mtau, +1, Dhel);
    }
    return (right - left) / (right + left);
  }

  double EvtBSemiTauonicDecayRateCalculator::PDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau)
  {
    double transverse(0), longitudinal(0);
    transverse += Gamma(BSTD, mtau, -1, -1);
    transverse += Gamma(BSTD, mtau, +1, -1);
    transverse += Gamma(BSTD, mtau, -1, +1);
    transverse += Gamma(BSTD, mtau, +1, +1);
    longitudinal += Gamma(BSTD, mtau, -1, 0);
    longitudinal += Gamma(BSTD, mtau, +1, 0);
    return longitudinal / (longitudinal + transverse);
  }

  double EvtBSemiTauonicDecayRateCalculator::pf(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau, int Dhel,
                                                double w)
  {
    return 1. / (2 * BSTD.getMB()) * BSTD.getMB() * BSTD.getMB() * BSTD.r(Dhel) * BSTD.r(Dhel)
           * BSTD.v(mtau, BSTD.q2(Dhel, w)) * BSTD.v(mtau, BSTD.q2(Dhel, w))
           * sqrt(w * w - 1) / (64 * M_PI * M_PI * M_PI);
  }

  double EvtBSemiTauonicDecayRateCalculator::EvaluateByW(double* x, double* param)
  {
    // x=w
    return dGammadwdcostau(*m_BSTD, param[0], (int)param[1], (int)param[2], x[0], param[3]);
  }

  double EvtBSemiTauonicDecayRateCalculator::EvaluateByCostau(double* x, double* param)
  {
    // x=costau
    return dGammadwdcostau(*m_BSTD, param[0], (int)param[1], (int)param[2], param[3], x[0]);
  }

  double EvtBSemiTauonicDecayRateCalculator::EvaluateBy2D(double* x, double* param)
  {
    // x={w,costau}
    return dGammadwdcostau(*m_BSTD, param[0], (int)param[1], (int)param[2], x[0], x[1]);
  }

} // namespace Belle2
