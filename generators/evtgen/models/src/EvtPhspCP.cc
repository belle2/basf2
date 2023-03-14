/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iomanip>
#include <cmath>
#include <string>

#include "EvtGenBase/EvtCPUtil.hh"
#include "EvtGenBase/EvtPDL.hh"

#include <generators/evtgen/EvtGenModelRegister.h>
#include "generators/evtgen/models/EvtPhspCP.h"

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtPhspCP);

  EvtPhspCP::~EvtPhspCP() {}

  std::string EvtPhspCP::getName()
  {
    return "PHSP_CP";
  }

  EvtDecayBase* EvtPhspCP::clone()
  {
    return new EvtPhspCP;
  }

  void EvtPhspCP::init()
  {
    // Check number of arguments
    checkNArg(7);
  }

  void EvtPhspCP::initProbMax()
  {
    setProbMax(2 * (getArg(3)*getArg(3) + getArg(5)*getArg(5)));
  }

  void EvtPhspCP::decay(EvtParticle* p)
  {
    static EvtId B0  = EvtPDL::getId("B0");
    static EvtId B0B = EvtPDL::getId("anti-B0");

    double t;
    EvtId other_b;

    EvtCPUtil::getInstance()->OtherB(p, t, other_b, 0.5);

    p->initializePhaseSpace(getNDaug(), getDaugs());

    EvtComplex amp;

    EvtComplex A, Abar;

    A    = EvtComplex(getArg(3) * cos(getArg(4)), getArg(3) * sin(getArg(4)));
    Abar = EvtComplex(getArg(5) * cos(getArg(6)), getArg(5) * sin(getArg(6)));

    // CP asymmetry
    const double angle = getArg(0); // q/p = exp(-2i*angle). |q/p| = 1
    const double dm    = getArg(1); // Delta m_d
    const double etaCP = getArg(2); // CP eigenvalue

    if (other_b == B0B) {
      amp = A * cos(dm * t / (2 * EvtConst::c)) +
            EvtComplex(cos(-2.0 * angle), sin(-2.0 * angle)) *
            etaCP * EvtComplex(0.0, 1.0) * Abar * sin(dm * t / (2 * EvtConst::c));

    }
    if (other_b == B0) {
      amp = A * EvtComplex(cos(2.0 * angle), sin(2.0 * angle)) *
            EvtComplex(0.0, 1.0) * sin(dm * t / (2 * EvtConst::c)) +
            etaCP * Abar * cos(dm * t / (2 * EvtConst::c));
    }

    vertex(amp);

    return;
  }

} // Belle 2 Namespace
