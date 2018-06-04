/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 Belle II Collaboration                               *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Markus Prim                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <generators/evtgen/models/EvtBCLFF.h>

#include <framework/logging/Logger.h>

#include <EvtGenBase/EvtId.hh>
#include <EvtGenBase/EvtPDL.hh>

#include <cmath>

namespace Belle2 {

  EvtBCLFF::EvtBCLFF(int numarg, double* arglist) : m_numBCLFFCoefficients(numarg)
  {
    for (int i = 0; i < m_numBCLFFCoefficients; ++i) {
      m_BCLFFCoefficients[i] = arglist[i];
    }
  }

  void EvtBCLFF::getscalarff(EvtId parent, EvtId daughter, double t, double, double* fpf, double* f0f)
  {

    if (m_numBCLFFCoefficients != 8) {
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Wrong number of arguments for EvtBCLFF::getscalarff!\n";
    }

    const auto mB = EvtPDL::getMeanMass(parent);
    const auto mM = EvtPDL::getMeanMass(daughter);

    const auto tplus = (mB + mM) * (mB + mM);
    const auto tzero = (mB + mM) * (std::sqrt(mB) - std::sqrt(mM)) * (std::sqrt(mB) - std::sqrt(mM));

    const auto mR2 = m_resonance1Minus * m_resonance1Minus;
    const auto pole = 1 / (1 - t / mR2);

    const std::vector<double> bplus = {m_BCLFFCoefficients[0], m_BCLFFCoefficients[1], m_BCLFFCoefficients[2], m_BCLFFCoefficients[3]};
    const std::vector<double> bzero = {m_BCLFFCoefficients[4], m_BCLFFCoefficients[5], m_BCLFFCoefficients[6], m_BCLFFCoefficients[7]};

    const auto N_fpf = bplus.size();
    const auto N_f0f = bzero.size();

    auto z = [tplus, tzero](decltype(t) q2) {
      const auto term1 = std::sqrt(tplus - q2);
      const auto term2 = std::sqrt(tplus - tzero);
      return (term1 - term2) / (term1 + term2);
    };

    double sum_fpf = 0;
    for (unsigned int n = 0; n < N_fpf; ++n) {
      sum_fpf += bplus[n] * (std::pow(z(t), n) - std::pow(-1, n - N_fpf) * n / N_fpf * std::pow(z(t), N_fpf));
    }
    *fpf = pole * sum_fpf;

    double sum_f0f = 0;
    for (unsigned int n = 0; n < N_f0f; ++n) {
      sum_f0f += bzero[n] * std::pow(z(t), n);
    }
    *f0f = sum_f0f;
  }

  void EvtBCLFF::getvectorff(EvtId parent, EvtId daughter, double t, double, double* a1f, double* a2f, double* vf, double* a0f)
  {

    if (m_numBCLFFCoefficients != 11) {
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Wrong number of arguments for EvtBCLFF::getvectorff!\n";
    }

    const auto mB = EvtPDL::getMeanMass(parent);
    const auto mB2 = mB * mB;
    const auto mM = EvtPDL::getMeanMass(daughter);
    const auto mM2 = mM * mM;

    const auto tplus = (mB + mM) * (mB + mM);
    const auto tminus = (mB - mM) * (mB - mM);
    const auto tzero = tplus * (1 - std::sqrt(1 - tminus / tplus));

    const auto mR2A0 = m_resonance0Minus * m_resonance0Minus;
    const auto mR2A1 = m_resonance1Plus * m_resonance1Plus;
    const auto mR2A12 = m_resonance1Plus * m_resonance1Plus;
    const auto mR2V = m_resonance1Minus * m_resonance1Minus;

    const auto poleA0 = 1. / (1 - t / mR2A0);
    const auto poleA1 = 1. / (1 - t / mR2A1);
    const auto poleA12 = 1. / (1 - t / mR2A12);
    const auto poleV = 1. / (1 - t / mR2V);

    const std::vector<double> A0 = {8 * mB* mM / (mB2 - mM2)* m_BCLFFCoefficients[5], m_BCLFFCoefficients[0], m_BCLFFCoefficients[1]};
    const std::vector<double> A1 = {m_BCLFFCoefficients[2], m_BCLFFCoefficients[3], m_BCLFFCoefficients[4]};
    const std::vector<double> A12 = {m_BCLFFCoefficients[5], m_BCLFFCoefficients[6], m_BCLFFCoefficients[7]};
    const std::vector<double> V = {m_BCLFFCoefficients[8], m_BCLFFCoefficients[9], m_BCLFFCoefficients[10]};

    auto z = [tplus, tzero](decltype(t) q2) {
      const auto term1 = std::sqrt(tplus - q2);
      const auto term2 = std::sqrt(tplus - tzero);
      return (term1 - term2) / (term1 + term2);
    };

    auto sum = [&z](decltype(t) q2, std::vector<double> par) {
      double tot = 0;
      for (unsigned int n = 0; n < par.size(); ++n) {
        tot += par[n] * std::pow(z(q2) - z(0), n);
      }
      return tot;
    };

    auto kaellen = [mB, mM](decltype(t) q2) {
      return ((mB + mM) * (mB + mM) - q2) * ((mB - mM) * (mB - mM) - q2);
    };

    const auto ffA0 = poleA0 * sum(t, A0);
    const auto ffA1 = poleA1 * sum(t, A1);
    const auto ffA12 = poleA12 * sum(t, A12);
    const auto ffV = poleV * sum(t, V);

    const auto ffA2 = ((mB + mM) * (mB + mM) * (mB2 - mM2 - t) * ffA1 - (16 * mB * mM2 * (mB + mM)) * ffA12) / kaellen(t);

    *a0f = ffA0;
    *a1f = ffA1;
    *a2f = ffA2;
    *vf = ffV;

  }

  void EvtBCLFF::gettensorff(EvtId, EvtId, double, double, double*, double*, double*, double*)
  {
    B2FATAL("Not implemented :gettensorff in EvtBCLFF.");
  }

  void EvtBCLFF::getbaryonff(EvtId, EvtId, double, double, double*, double*, double*, double*)
  {
    B2FATAL("Not implemented :getbaryonff in EvtBCLFF.");
  }

  void EvtBCLFF::getdiracff(EvtId, EvtId, double, double, double*, double*, double*, double*, double*, double*)
  {
    B2FATAL("Not implemented :getbaryonff in EvtBCLFF.");
  }

  void EvtBCLFF::getraritaff(EvtId, EvtId, double, double, double*, double*, double*, double*, double*, double*, double*, double*)
  {
    B2FATAL("Not implemented :getbaryonff in EvtBCLFF.");
  }
}