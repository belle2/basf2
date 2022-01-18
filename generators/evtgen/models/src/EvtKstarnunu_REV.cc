/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/evtgen/EvtGenModelRegister.h>

#include "EvtGenBase/EvtPatches.hh"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cmath>
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtDiracSpinor.hh"
#include "EvtGenBase/EvtTensor4C.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtVector4C.hh"

#include "generators/evtgen/models/EvtKstarnunu_REV.h"

using std::endl;

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtKstarnunu_REV);

  EvtKstarnunu_REV::~EvtKstarnunu_REV() { }

  std::string EvtKstarnunu_REV::getName()
  {
    return "KSTARNUNU_REV";
  }

  EvtDecayBase* EvtKstarnunu_REV::clone()
  {
    return new EvtKstarnunu_REV;
  }

  void EvtKstarnunu_REV::decay(EvtParticle* p)
  {

    static EvtId NUE = EvtPDL::getId("nu_e");
    static EvtId NUM = EvtPDL::getId("nu_mu");
    static EvtId NUT = EvtPDL::getId("nu_tau");
    static EvtId NUEB = EvtPDL::getId("anti-nu_e");
    static EvtId NUMB = EvtPDL::getId("anti-nu_mu");
    static EvtId NUTB = EvtPDL::getId("anti-nu_tau");

    p->initializePhaseSpace(getNDaug(), getDaugs());

    double m_b = p->mass();

    EvtParticle* meson, * neutrino1, * neutrino2;
    meson = p->getDaug(0);
    neutrino1 = p->getDaug(1);
    neutrino2 = p->getDaug(2);
    EvtVector4R momnu1 = neutrino1->getP4();
    EvtVector4R momnu2 = neutrino2->getP4();
    EvtVector4R momkstar = meson->getP4();

    EvtVector4R q = momnu1 + momnu2;
    double q2 = q.mass2();

    double m_k = meson->mass();

    EvtVector4R p4b; p4b.set(m_b, 0., 0., 0.);  // Do calcs in mother rest frame

    // calculate form factors [arXiv:1503.05534v3]
    // see Table 15, eq 15, eq 16, Table 3

    // FFs
    double tp = (m_b + m_k) * (m_b + m_k);
    double tm = (m_b - m_k) * (m_b - m_k);
    double t0 = tp * (1 - sqrt(1 - tm / tp));
    double z = (sqrt(tp - q2) - sqrt(tp - t0)) / (sqrt(tp - q2) + sqrt(tp - t0));
    double z0 = (sqrt(tp) - sqrt(tp - t0)) / (sqrt(tp) + sqrt(tp - t0));

    // v0
    double alpha0_v0 = 0.38;
    double alpha1_v0 = -1.17;
    double alpha2_v0 = 2.42;
    double mR_v0 = 5.415;
    double v0 = (1 / (1 - q2 / (mR_v0 * mR_v0))) * (alpha0_v0 + alpha1_v0 * (z - z0) + alpha2_v0 * (z - z0) * (z - z0));

    // A1
    double alpha0_A1 = 0.3;
    double alpha1_A1 = 0.39;
    double alpha2_A1 = 1.19;
    double mR_A1 = 5.829;
    double A1 = (1 / (1 - q2 / (mR_A1 * mR_A1))) * (alpha0_A1 + alpha1_A1 * (z - z0) + alpha2_A1 * (z - z0) * (z - z0));

    // A12
    double alpha0_A12 = 0.27;
    double alpha1_A12 = 0.53;
    double alpha2_A12 = 0.48;
    double mR_A12 = 5.829;
    double A12 = (1 / (1 - q2 / (mR_A12 * mR_A12))) * (alpha0_A12 + alpha1_A12 * (z - z0) + alpha2_A12 * (z - z0) * (z - z0));

    // lambda
    double lambda = (tp - q2) * (tm - q2);

    // A2
    double A2 = ((m_b + m_k) * (m_b + m_k) * (m_b * m_b - m_k * m_k - q2) * A1 - A12 * 16 * m_b * m_k * m_k * (m_b + m_k)) / lambda;

    // calculate quark decay amplitude from [arXiv:hep-ph/9910221v2]
    // see eq 3.3, eq 3.4, eq 3.5, eq 4.1, eq 4.4, and eq 4.5
    // but in B->Kstar nu nubar, A3, A0, and all T terms are ignored

    // definition of A12 can be found from [arXiv:1303.5794v2]

    // definition of A3 can be found from [arXiv:1408.5614v1]

    EvtTensor4C tds = (-2 * v0 / (m_b + m_k)) * dual(EvtGenFunctions::directProd(p4b, momkstar))
                      - EvtComplex(0.0, 1.0) *
                      ((m_b + m_k) * A1 * EvtTensor4C::g()
                       - (A2 / (m_b + m_k)) * EvtGenFunctions::directProd(p4b - momkstar, p4b + momkstar));

    EvtVector4C l;

    if (getDaug(1) == NUE || getDaug(1) == NUM || getDaug(1) == NUT) {
      l = EvtLeptonVACurrent(neutrino1->spParentNeutrino(),
                             neutrino2->spParentNeutrino());
    }
    if (getDaug(1) == NUEB || getDaug(1) == NUMB || getDaug(1) == NUTB) {
      l = EvtLeptonVACurrent(neutrino2->spParentNeutrino(),
                             neutrino1->spParentNeutrino());
    }

    EvtVector4C et0, et1, et2;
    et0 = tds.cont1(meson->epsParent(0).conj());
    et1 = tds.cont1(meson->epsParent(1).conj());
    et2 = tds.cont1(meson->epsParent(2).conj());

    vertex(0, l * et0);
    vertex(1, l * et1);
    vertex(2, l * et2);

    return;

  }

  void EvtKstarnunu_REV::init()
  {

    // check that there are 0 arguments
    checkNArg(0);
    checkNDaug(3);

    //We expect the parent to be a scalar
    //and the daughters to be Kstar neutrino netrino

    checkSpinParent(EvtSpinType::SCALAR);

    checkSpinDaughter(0, EvtSpinType::VECTOR);
    checkSpinDaughter(1, EvtSpinType::NEUTRINO);
    checkSpinDaughter(2, EvtSpinType::NEUTRINO);
  }

  void EvtKstarnunu_REV::initProbMax()
  {
    // Maximum probability was obtained by 10^6 MC samples. It was 20000.169
    // About 10% higher value is used.
    setProbMax(22000.0);
  }

}
